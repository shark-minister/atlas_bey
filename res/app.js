const { createApp } = Vue

// constants
const ATLAS_SERVICE ="32150000-9a86-43ac-b15f-200ed1b7a72a";

class ElectricLauncher {
    constructor() {
        this.enabled_manual = false;
        this.rot_is_right = true;
        this.sp = 10000;
    }
    get_flag() {
        let result = 0;
        if (this.enabled_manual) {
            result |= 1;
        }
        if (!this.rot_is_right) {
            result |= (1 << 4);
        }
        return result;
    }
    set_flag(value) {
        this.enabled_manual = ((value & 1) > 0);
        this.rot_is_right = ((value & 16) == 0);
    }
}

const app = Vue.createApp({
    data() {
        return {
            // パラメータ
            elr2_auto: false,
            sp_meas_only: true,
            switch_less: false,
            is_bbp_sp_main: false,
            num_motors: 1,
            latency: 2000,
            delay: 0,
            elr1: new ElectricLauncher(),
            elr2: new ElectricLauncher(),
            write_rom: false,
            // Bluetooth
            device: null,
            service: null,
            last_uuid: "",
            last_characteristic: null,
            is_gatt_busy: false,
            // データ
            total: 0,
            min_sp: 0,
            max_sp: 0,
            avg_sp: 0,
            std_sp: 0,
            chart: null
        }
    },
    computed: {
        is_button_busy() {
            return this.device === null || this.is_gatt_busy;
        }
    },
    methods: {
        //---------------------------------------------------------------------
        // データ
        //---------------------------------------------------------------------
        serialize_params() {
            return new Uint8Array([
                this.is_bbp_sp_main ? 8 : 0,
                this.latency / 10,
                this.delay / 2,
                this.elr1.get_flag(),
                this.elr1.sp / 100,
                this.elr2.get_flag(),
                this.elr2.sp / 100
            ]);
        },
        deserialize_params(data) {
            const first_byte = data.getUint8(0);
            // 0000 0001 (1)
            this.elr2_auto      = (first_byte & 1) > 0;
            // 0000 0010 (2)
            this.sp_meas_only   = (first_byte & 2) > 0 ? true : false;
            // 0000 0100 (4)
            this.switch_less    = (first_byte & 4) > 0 ? true : false;
            // 0000 1000 (8)
            this.is_bbp_sp_main = (first_byte & 8) > 0 ? true : false;
            // 0001 0000 (16)
            this.num_motors     = (first_byte & 16) > 0 ? 2 : 1;

            this.latency = data.getUint8(1) * 10;
            this.delay = data.getUint8(2) * 2;
            this.elr1.set_flag(data.getUint8(3));
            this.elr1.sp = data.getUint8(4) * 100;
            this.elr2.set_flag(data.getUint8(5));
            this.elr2.sp = data.getUint8(6) * 100;    
        },
        deserialize_data(header, hists) {
            // ヘッダ情報
            this.total = header.getUint16(0, true);
            this.max_sp = header.getUint16(2, true);
            this.min_sp = header.getUint16(4, true);
            this.avg_sp = header.getUint16(6, true);
            this.std_sp = header.getUint16(8, true);
            this.hist_begin = header.getUint8(10);
            this.hist_end = header.getUint8(11);

            if (this.total > 0) {
                // ヒストグラムのビン数
                const hist_n_bins = this.hist_end - this.hist_begin + 1;
                // ヒストグラム
                this.label = new Array(hist_n_bins);
                this.data = new Array(hist_n_bins);

                for (let i = this.hist_begin; i <= this.hist_end; i += 1) {
                    const block = Math.floor(i / 20);
                    const index = i - (block * 20);
                    this.data[i - this.hist_begin] = hists[block].getUint8(index);
                    this.label[i - this.hist_begin] = String(4000 + i * 200);
                }
                return true;
            }
            else {
                this.label = []
                this.data = []
                return false;
            }
        },
        plot() {
            if (this.chart) {
                this.chart.destroy();
            }
            const ctx = document.getElementById("histogram").getContext('2d');
            this.chart = new Chart(ctx, {
                type: "bar",
                data: {
                    labels: this.label,
                    datasets: [{
                        data: this.data,
                        backgroundColor: "#008fd1",
                    }],
                },
                options: {
                    indexAxis: 'y',
                    responsive: true,
                    scales: {
                        x: {
                            title: {
                                display: true,
                                font: {
                                    size: 18
                                },
                                text: "シュート数"
                            }
                        },
                        y: {
                            reverse: true,
                            title: {
                                display: true,
                                font: {
                                    size: 18
                                },
                                text: "シュートパワー"
                            }
                        }
                    },
                    plugins: {
                        title: {
                            display: true,
                            font: {
                                size: 20,
                            },
                            text: "シュートパワー分布（4000以上）"
                        },
                        legend: {
                            display: false
                        },
                    },
                }
            });
        },
        //---------------------------------------------------------------------
        // Bluetooth Low Energy
        //---------------------------------------------------------------------
        async connect() {
            // デバイスが見つかっていなければ
            if (this.device == null) {
                // スキャン実行
                console.log("finding device...");
                this.is_gatt_busy = true;
                this.device = await navigator.bluetooth.requestDevice({
                    filters: [{
                        // サービス名は固定
                        services: [ATLAS_SERVICE],
                    }]
                });
                console.log("finding device...done");
            }
            
            // GATTサービスに接続済みであれば
            if (this.device.gatt.connected && this.service) {
                return;
            }

            // デバイスが切断されたときのコールバックを登録
            this.device.addEventListener('gattserverdisconnected', (event) => {
                const dev = event.target;
                console.log(`device ${dev.name} is disconnected.`);
                this.sp_meas_only = true;
            });

            // 接続実行
            console.log("connecting...");
            this.is_gatt_busy = true;
            const server = await this.device.gatt.connect();
            this.service = await server.getPrimaryService(ATLAS_SERVICE);
            this.is_gatt_busy = false;
            console.log("connecting...done");
        },
        async get_characteristic(uuid) {
            console.log('getting characteristic...');
            if (this.last_uuid != uuid) {
                this.is_gatt_busy = true;
                this.last_characteristic = await this.service.getCharacteristic(uuid);
                this.is_gatt_busy = false;
            }
            return this.last_characteristic;
        },
        async read(uuid)
        {
            try {
                await this.connect();
                console.log("reading characteristic...");
                this.is_gatt_busy = true;
                const characteristic = await this.get_characteristic(uuid);
                const data = await characteristic.readValue();
                this.is_gatt_busy = false;
                console.log("reading characteristic...done");
                return data;
            }
            catch (error) {
                console.log("error: " + error);
            }
        },
        async write(uuid, value)
        {
            try {
                await this.connect();
                console.log("writing characteristic...");
                this.is_gatt_busy = true;
                const characteristic = await this.get_characteristic(uuid);
                await characteristic.writeValue(value);
                this.is_gatt_busy = false;
                console.log("writing characteristic...done");
                return;
            }
            catch (error) {
                console.log("error: " + error);
            }
        },
        async disconnect() {
            if (this.device == null) {
                return;
            }
            if (this.device.gatt.connected) {
                console.log("disconnecting...");
                this.is_gatt_busy = true;
                await this.device.gatt.disconnect();
                this.is_gatt_busy = false;
                console.log("disconnecting...done");
            }
            this.device = null;
        },
        //---------------------------------------------------------------------
        // Actions
        //---------------------------------------------------------------------
        async connect_to_atlas() {
            await this.connect();
            await this.read_params();
        },
        async disconnect_from_atlas() {
            await this.disconnect();
        },
        async read_params() {
            console.log("reading params...");
            const data = await this.read("32150001-9a86-43ac-b15f-200ed1b7a72a");
            this.deserialize_params(data);
        },
        async write_params() {
            console.log("writing params...");
            // 値が有効な場合のみ
            if (document.getElementById("elr1-sp").checkValidity() &&
                (this.num_motors == 1 || document.getElementById("elr2-sp").checkValidity()) &&
                document.getElementById("delay").checkValidity() &&
                document.getElementById("latency").checkValidity())
            {
                await this.write(
                    "32150001-9a86-43ac-b15f-200ed1b7a72a",
                    this.serialize_params()
                );
                /*
                if (this.write_rom) {
                    // 本体ROMへパラメータを記憶させる
                    await this.store_params();
                }
                */
                return;
            }
            alert("値が適切ではありません");
        },
        /*
        async store_params() {
            console.log("storing params in flash memory...");
            await this.write(
                "32150010-9a86-43ac-b15f-200ed1b7a72a",
                new Uint8Array([1])
            );
        },
        */
        async launch() {
            console.log("launching beyblade...");
            await this.write(
                "32150020-9a86-43ac-b15f-200ed1b7a72a",
                new Uint8Array([1])
            );
        },
        async read_shoot_data() {
            console.log("reading header...");
            const headr = await this.read("32150030-9a86-43ac-b15f-200ed1b7a72a");
            const hists = [null, null, null];
            for (let i = 0; i < 3; i += 1) {
                console.log("reading data...");
                hists[i] = await this.read("32150031-9a86-43ac-b15f-200ed1b7a72a");
            }
            const result = this.deserialize_data(headr, hists);
            console.log("reading data...done");
            if (result) {
                this.plot();
            }
        },
        async clear_shoot_data() {
            if (confirm("コントローラ内のデータを初期化しますか？")) {
                console.log("clearing data...");
                await this.write(
                    "32150040-9a86-43ac-b15f-200ed1b7a72a",
                    new Uint8Array([1])
                );
                console.log("clearing data...done");
            }
        },
        async switch_to_automode() {
            if (confirm("コントローラを計測モード(A)に切り替えますか？")) {
                console.log("switching...");
                await this.write(
                    "32150050-9a86-43ac-b15f-200ed1b7a72a",
                    new Uint8Array([1])
                );
                console.log("switching...done");
            }
        }
    }
})
app.mount('#atlas-client');

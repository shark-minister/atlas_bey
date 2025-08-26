const { createApp } = Vue

// constants
const ATLAS_SERVICE ="32150000-9a86-43ac-b15f-200ed1b7a72a";

/*
    電動ランチャーの設定を保持するクラス
*/
class ElectricLauncher {
    // コンストラクタ
    constructor() {
        // マニュアルモードで利用するか否か
        this.enabled_manual = false;
        // 右回転か否か。falseなら左回転
        this.rot_is_right = true;
        // 設定SP
        this.sp = 10000;
    }
    serialize_flag() {
        let result = 0;
        if (this.enabled_manual) {
            result |= 1;
        }
        if (!this.rot_is_right) {
            result |= (1 << 4);
        }
        return result;
    }
    deserialize_flag(value) {
        this.enabled_manual = ((value & 1) > 0);
        this.rot_is_right = ((value & 16) == 0);
    }
}

const app = Vue.createApp({
    data() {
        return {
            //-----------------------------------------------------------------
            // デバイス実装情報
            //-----------------------------------------------------------------
            /*
                バージョン（16ビット整数）
                - 0xF000: メジャーバージョン
                - 0x0F00: マイナーバージョン
                - 0x00FF: リビジョン
            */
            version: 0,
            /*
                利用形態。SP計測器としての実装か否か
                - 0: 電動ランチャー制御器
                - 1: SP測定器
           */
            format: 1,
            /*
                スイッチレスか否か
                - True:  スイッチなし
                - False: スイッチあり
            */
            switch_less: false,
            // 使用するモーター数
            num_elrs: 1,
            // モーター1の最大回転数
            elr1_max_rpm: 0,
            // モーター2の最大回転数
            elr2_max_rpm: 0,
            //-----------------------------------------------------------------
            // パラメータ
            //-----------------------------------------------------------------
            /*
                オートモードで利用する電動ランチャー
                - False: ELR 1
                - True:  ELR 2
            */
            elr2_auto: false,
            // オートモードにて、表示するSP値の種類
            // - True:  BBPの値をメインに表示する
            // - False: 真のSP値をメインに表示する
            is_bbp_sp_main: false,
            latency: 2000,
            delay: 0,
            elr1: new ElectricLauncher(),
            elr2: new ElectricLauncher(),
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
        // ボタンが押せる状態かどうか
        is_button_busy() {
            return this.device === null || this.is_gatt_busy;
        },
        // デバイスがSP計測器オンリーの実装か
        is_sp_meas_only() {
            return this.format === 1;
        },
        // バージョン表記
        version_str() {
            return (
                (this.version >> 12).toString() + "." +
                ((this.version >> 8) & 0xF).toString() + "." +
                (this.version & 0xFF).toString()
            );
        }
    },
    methods: {
        //---------------------------------------------------------------------
        // Bluetooth Low Energy
        //---------------------------------------------------------------------
        // 接続
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
                this.format = 1;
            });

            // 接続実行
            console.log("connecting...");
            this.is_gatt_busy = true;
            const server = await this.device.gatt.connect();
            this.service = await server.getPrimaryService(ATLAS_SERVICE);
            this.is_gatt_busy = false;
            console.log("connecting...done");
        },
        // キャラクタリスティックの取得
        async get_characteristic(uuid) {
            console.log('getting characteristic...');
            if (this.last_uuid != uuid) {
                this.is_gatt_busy = true;
                this.last_characteristic = await this.service.getCharacteristic(uuid);
                this.is_gatt_busy = false;
            }
            return this.last_characteristic;
        },
        // キャラクタリスティックの読み出し
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
        // キャラクタリスティックの書き込み
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
        // 切断
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
        // ATLASに接続する
        async connect_to_atlas() {
            await this.connect();
            await this.check_version();
            await this.read_params();
        },
        // ATLASから切断する
        async disconnect_from_atlas() {
            await this.disconnect();
        },
        // ATLASのバージョンを確認する
        async check_version() {
            // デフォルトのバージョン
            this.version = 0x1100;
            // デバイスのキャラクタリスティック一覧を取得
            const chars = await this.service.getCharacteristics();
            for (let i = 0; i < chars.length; ++i) {
                // キャラクタリスティックのUUIDを取得
                const uuid = chars[i].uuid;
                console.log(uuid);

                // ver.1.2.0 以降かどうかの判断
                // デバイス情報を読み取れるキャラクタリスティックの実装
                if (uuid == "32150060-9a86-43ac-b15f-200ed1b7a72a") {
                    // デバイス情報を取得する
                    const characteristic = await this.get_characteristic(uuid);
                    const data = await characteristic.readValue();

                    // バージョン取得（リトルエンディアン）
                    this.version = data.getUint16(0, true);

                    // コンディション取得（リトルエンディアン）
                    const cond = data.getUint16(2, true);
                    /*
                        実装フォーマット（マスク：0000 0011） 
                        - `0`: 電動ランチャー制御器
                        - `1`: SP測定器
                        - `2`: 予備
                        - `3`: 予備
                    */
                    this.format = (cond & 0b11);
                    /*
                        スイッチレスか否か（マスク：0000 0100） 
                        - false: スイッチあり
                        - true: スイッチなし
                    */
                    this.switch_less = (cond & 0b100) > 0 ? true : false;
                    /*
                        電動ランチャーは2台構成か否か（マスク：0001 1000） 
                        - `0`: モータは1台構成
                        - `1`: モータは2台構成
                        - `2`: 予備
                        - `3`: 予備
                    */
                    this.num_elrs = ((cond & 0b11000) >> 3) + 1;

                    // モーターの最大回転数
                    this.elr1_max_rpm = data.getUint8(4);
                    this.elr2_max_rpm = data.getUint8(5);
                }
                // パラメータをROMに書き込むかどうかを設定するためのキャラクタリスティック
                // ver. 1.1.0 以前に実装されていて、現在は廃止
                else if (uuid == "32150010-9a86-43ac-b15f-200ed1b7a72a") {
                    this.version = 0x1000;
                }
            }
            console.log("version == 0x" + this.version.toString(16));
        },
        // パラメータを読み出す
        async read_params() {
            console.log("reading params...");
            const data = await this.read("32150001-9a86-43ac-b15f-200ed1b7a72a");
            
            // 先頭の1バイトを取得
            const first_byte = data.getUint8(0);
            // 0000 0001
            this.elr2_auto = (first_byte & 0b1) > 0;
            // 0000 1000
            this.is_bbp_sp_main = (first_byte & 0b1000) > 0;

            // Ver.1.2未満には、デバイス情報を取得するキャラクタリスティックがない。
            // したがって、必要な情報はパラメータから取得する。
            // これらの値は読み込み専用
            if (this.version < 0x1200) {
                console.log("version < 0x1200: read format, switch less, #elrs from params");
                // 0000 0010
                this.format = (first_byte & 0b10) > 0 ? 1 : 0;
                // 0000 0100
                this.switch_less = (first_byte & 0b100) > 0;
                // 0001 0000
                this.num_elrs = (first_byte & 0b10000) > 0 ? 2 : 1;
            }

            // 射出猶予時間 [ms]
            // 値は 1/10 で送られてくる
            this.latency = data.getUint8(1) * 10;
            // ベイ射出遅延時間 [ms]
            // 値は 1/2 で送られてくる
            this.delay = data.getUint8(2) * 2;
            // 電動ランチャー1の設定
            this.elr1.deserialize_flag(data.getUint8(3));
            // 電動ランチャー1のSP値 [rpm]
            // 値は 1/100 で送られてくる
            this.elr1.sp = data.getUint8(4) * 100;
            // 電動ランチャー2の設定
            this.elr2.deserialize_flag(data.getUint8(5));
            // 値は 1/100 で送られてくる
            // 電動ランチャー2のSP値 [rpm]
            this.elr2.sp = data.getUint8(6) * 100;   
        },
        // パラメータを書き込む
        async write_params() {
            console.log("writing params...");
            // 値が有効な場合のみ
            if (document.getElementById("elr1-sp").checkValidity() &&
                (this.num_elrs == 1 || document.getElementById("elr2-sp").checkValidity()) &&
                document.getElementById("delay").checkValidity() &&
                document.getElementById("latency").checkValidity())
            {
                const flag = (this.elr2_auto ? 0b1 : 0) |
                             (this.is_bbp_sp_main ? 0b1000 : 0);
                await this.write(
                    "32150001-9a86-43ac-b15f-200ed1b7a72a",
                    new Uint8Array([
                        flag,
                        this.latency / 10,
                        this.delay / 2,
                        this.elr1.serialize_flag(),
                        this.elr1.sp / 100,
                        this.elr2.serialize_flag(),
                        this.elr2.sp / 100
                    ])
                );
                // Ver.1.0.0のみ本体ROMへパラメータを記録させる
                // Ver.1.1.0以降のバージョンでは、"32150001-9a86-43ac-b15f-200ed1b7a72a" の中で記録される
                if (this.version === 0x1000) {
                    console.log("storing params in flash memory...");
                    await this.write(
                        "32150010-9a86-43ac-b15f-200ed1b7a72a",
                        new Uint8Array([1])
                    );
                }
                return;
            }
            alert("値が適切ではありません");
        },
        // マニュアル射出
        async launch() {
            console.log("launching beyblade...");
            await this.write(
                "32150020-9a86-43ac-b15f-200ed1b7a72a",
                new Uint8Array([1])
            );
        },
        // シュートデータの読み出し
        async read_shoot_data() {
            // ヘッダ情報
            console.log("reading header...");
            const headr = await this.read("32150030-9a86-43ac-b15f-200ed1b7a72a");
            const hists = [null, null, null];
            // ヒストグラムデータ（3回読む）
            for (let i = 0; i < 3; i += 1) {
                console.log("reading data...");
                hists[i] = await this.read("32150031-9a86-43ac-b15f-200ed1b7a72a");
            }

            // ヘッダ情報（全てリトルエンディアン）
            this.total = headr.getUint16(0, true);
            this.max_sp = headr.getUint16(2, true);
            this.min_sp = headr.getUint16(4, true);
            this.avg_sp = headr.getUint16(6, true);
            this.std_sp = headr.getUint16(8, true);
            this.hist_begin = headr.getUint8(10);
            this.hist_end = headr.getUint8(11);

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
            }
            else {
                this.label = []
                this.data = []
            }

            console.log("reading data...done");
            await this.plot();
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
        },
        async plot() {
            if (this.chart) {
                this.chart.destroy();
            }
            if (this.total == 0) {
                return;
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
        }
    }
})
app.mount('#atlas-client');

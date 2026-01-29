/*
    電動ランチャーの設定を保持するクラス
*/
export class ElectricLauncher {
    // マニュアルモードで利用するか否か
    enabledManual: boolean = false
    // 右回転か否か。falseなら左回転
    isRotRight: boolean = true
    // シュートパワー
    sp: number = 10000
    // 直列化
    serializeFlag() {
        let result = 0
        if (this.enabledManual) {
            result |= 1
        }
        if (!this.isRotRight) {
            result |= (1 << 4)
        }
        return result
    }
    deserializeFlag(value: number) {
        this.enabledManual = ((value & 1) > 0)
        this.isRotRight = ((value & 16) == 0)
    }
}

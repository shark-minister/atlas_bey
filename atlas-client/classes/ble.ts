// キャラクタリスティックの存在を確認する
export async function tryGetCharacteristic(
    service: BluetoothRemoteGATTService,
    uuid: BluetoothCharacteristicUUID
): Promise<BluetoothRemoteGATTCharacteristic | null> {
    try {
        return await service.getCharacteristic(uuid)
    } catch {
        return null
    }
}

// キャラクタリスティックからの読み出し
export async function readCharacteristic(
    service: BluetoothRemoteGATTService,
    uuid: BluetoothCharacteristicUUID
): Promise<DataView<ArrayBufferLike> | null>
{
    try {
        console.log("reading characteristic...")
        const ch = await service.getCharacteristic(uuid)
        const data = await ch.readValue()
        console.log("reading characteristic...done")
        return data
    }
    catch (error) {
        console.log("error: " + error);
    }
    return null
}

// キャラクタリスティックへの書き込み
export async function writeCharacteristic(
    service: BluetoothRemoteGATTService,
    uuid: BluetoothCharacteristicUUID,
    value: Uint8Array<ArrayBuffer>
): Promise<boolean>
{
    try {
        console.log("writing characteristic...")
        const ch = await service.getCharacteristic(uuid)
        await ch.writeValue(value)
        console.log("writing characteristic...done")
        return true
    }
    catch (error) {
        console.log("error: " + error);
    }
    return false
}
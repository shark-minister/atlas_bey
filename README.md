ベイブレードXの**マイコン制御電動ランチャーシステム ATLAS**および**シュートパワー計測器 ATLAS lite**のソースコードです。以下の点にご留意ください。

- 本稿の図は、ソフトウェアの画面キャプチャを除き、すべて私 "さめ大臣 / shark minister" が実物を元に書き起こしたものです。これらの図の、無断での複製や配布、改変などはご遠慮ください。
- 本稿を参考に製作や運転をされる場合は、自己責任のもとで実施お願いします。それによって生じたトラブル等については、いかなる責任も負いかねます。
- 私 "さめ大臣 / shark minister" は電子工作・電気工作の素人につき、ところどころ間違っている恐れがあります。プログラミングも趣味の域を超えません。全てを鵜呑みにするのは危険かもしれませんので、その点をご注意くだされば幸いです。

**スライドスイッチを用いた機体で、使用マニュアルを見たい方は ver.1.2.4 のドキュメントを参照してください。** <br/>
ver.1.2.4 <br/>
https://github.com/shark-minister/atlas_bey/tree/v1.2.4

# 1. はじめに

マイコン制御電動ランチャーシステムATLAS（**A**u**T**omatic **LA**unching **S**ystem）は、タカラトミーが販売するベイブレードXの遊び・練習を充実させるため、電動ランチャー（モーターによるベイブレード射出機）をマイコンで制御し、リアルのシュートジムを実現してみようと開発したシステムです。

ATLASでは、ランチャーにベイをセットしたら自動で検知してカウントダウン（3, 2, 1, ゴー、シュート！）を行ってからベイブレード射出を行う「オートモード」と、電動ランチャーのシュートパワー（SP）などの設定をPCやスマートフォンから行ったり、手動でベイブレード射出を行ったりできる「マニュアル・設定モード」の2つのモードがあります。

ATLASの概略図を下に示します。

<p align="center">
<img width="900" alt="マイコン制御電動ランチャー" src="https://github.com/user-attachments/assets/eed27498-140a-43d8-93f9-0c9e69303b3e" />
</p>

また、電動ランチャー制御機能を削除してシュートパワー計測のみに特化した派生版**ATLAS lite**もあります。
ATLASとは同一のソースコードであり、プログラムビルドの際に設定を変えるだけです（3-4-2節「機器の利用形態（共通）」参照）。

<p align="center">
<img width="750" alt="SP計測機" src="https://github.com/user-attachments/assets/6f0721ff-01c9-4857-af36-9ef2e0d479f0" />
</p>


# 2. 使い方

## 2-1. 給電と起動

ATLASのマイコンへはUSB-Cで電力を供給します。
お手持ちのACアダプタ、モバイルバッテリー、スマートフォン、iPad等で給電してください。
立ち上がると1～2秒スプラッシュスクリーンが表示され、オートモードの画面が表示されます。
モード切替は押しボタンスイッチを長押しして行います。

## 2-2. オートモード (A)　SP計測器の場合は「計測モード」

### 2-2-1. 概要

オートモードは、主に「人」vs「電動ランチャー」で遊ぶモードです。
ベイバトルパスを装着したランチャーにベイをセットするとそれを検知して自動でカウントダウン（3, 2, 1, ゴー、シュート！）を行い、電動ランチャーにセットされたベイが射出されます。

また、モーターを連動させずに単なる**シュートパワー計測機**としても使うことができます。

**機能**

- ベイバトルパスと連動
  - ブレーダーのシュートパワーを取得・表示
- 電動ランチャーによる自動射出
  - 自分のランチャーにベイをセットすることでカウントダウンを開始し射出

オートモードでは**ベイバトルパス**が必須となります。

> タカラトミー BX-09 ベイバトルパス  
> https://beyblade.takaratomy.co.jp/beyblade-x/lineup/bx09.html

### 2-2-2. ベイバトルパスの接続・切断

電源を入れるとオートモードで起動し、ベイバトルパス（以下、BBP）の接続を促すメッセージが表示されます。

<p align="center">
<img width="200" alt="オートモード画面：バトルパス接続促進" src="https://github.com/user-attachments/assets/a9715cdc-abd0-4034-8c4e-f2a7be9f517f" />
</p>

BBPの電源を投入し、BBPの天面ボタンを長押しすることで接続依頼（Bluetoothのアドバタイズ）を行います。ATLASがそれを検知し接続が完了するとBBPのインジケータが緑色の点滅を行います（スマホのベイブレードアプリへの接続と同じです）。また、その際に画面表示が変わって、画面上部のヘッダ領域にBBP接続アイコンが表示されます。

<p align="center">
<img width="200" alt="オートモード画面：バトルパス接続済み" src="https://github.com/user-attachments/assets/4fa39b84-e534-491a-b797-e4527433c38f" />
</p>

BBPを切断したいときはBBPのボタンを長押ししてください。インジケータが赤色の点滅を行ったら切断成功です。ヘッダ領域のBBP接続アイコンが消えることを確認してください。

### 2-2-3. シュートパワー計測のみの利用

オートモード起動直後は、安全のためモーターが無効にされています。この状態では、電動ランチャーを連動させずに単なるシュートパワー計測機として使うことができます。ベイをランチャー（BBPは接続済み）にセットするとATLASはそれを感知し、画面上部のヘッダ領域にベイブレードアイコンがつきます。ベイを外すとベイブレードアイコンは消えます。

<p align="center">
<img width="200" alt="オートモード画面：ベイ装着済み" src="https://github.com/user-attachments/assets/34c79ada-3c26-49f0-a4c9-c6e18b67a35b" />
</p>

ベイのシュートを行うと、画面にシュートパワーと合計シュート数（下の例だと、#123）が表示されます。

<p align="center">
<img width="200" alt="オートモード画面" src="https://github.com/user-attachments/assets/05eb9320-2f63-43f0-924d-7fff0c66e269" />
</p>

ここで表示されるシュートパワーは、以下の3つです。

- **YOUR SP**: ベイバトルパスに記録されたシュートパワー値（アプリで読み込まれるのはこの値）。? が付記され、かつストリングランチャーを使用している場合は、異常値である可能性が高いです。
- **EST**: 異常値を除外した推定シュートパワー値。ワインダーランチャーの場合は、0となる可能性が高いです。
- **MAX**: ランチャー回転毎の瞬間シュートパワーリスト（シュートパワープロファイル）の中の最も高いシュートパワー値

2-3-3節で説明する設定により、「ベイバトルパスに記録されたシュートパワー値」と「異常値を除外した推定シュートパワー値」を逆の表示位置にすることができます。その場合、以下のような画面になります。

<p align="center">
<img width="200" alt="auto_sp_rev" src="https://github.com/user-attachments/assets/b75573f9-2bda-4fca-a60e-34b4e847b555" />
</p>

ここで表示されるシュートパワーは、以下の3つです。

- **EST. SP**: 異常値を除外した推定シュートパワー値。ワインダーランチャーの場合は、0となる可能性があります。
- **BBP**: ベイバトルパスに記録されたシュートパワー値（アプリで読み込まれるのはこの値）。? が付記され、かつストリングランチャーを使用している場合は、異常値である可能性が高いです。
- **MAX**: ランチャー回転毎の瞬間シュートパワーリスト（シュートパワープロファイル）の中の最も高いシュートパワー値

ベイバトルパスからのデータが正しく送受信されないとチェックサムエラーとなり、画面にCRC ERRORと表示されます。

<p align="center">
<img width="200" alt="オートモード画面：CRCエラー" src="https://github.com/user-attachments/assets/78441be7-4df7-4c0a-9faa-0c76f041d430" />
</p>

### 2-2-4. 電動ランチャーの利用

電動ランチャーの射出するベイとバトルしたい場合はモーターを有効にする必要があります。モーターを有効にするには、ランチャーにベイがセットされていない状態でBBPのボタンを2連続で押してください（ダブルクリック）。モーターが有効になるとヘッダ領域に電動ランチャーの情報が表示されます。下の図の例だと、電動ランチャー1が有効です。

<p align="center">
<img width="200" alt="オートモード画面：電動ランチャー有効" src="https://github.com/user-attachments/assets/d0a3a6a9-5eb0-4a8f-88da-849c9537565a" />
</p>

この状態でベイをセットすると、ディスプレイに**ReadySet**と大きく表示されます。

<p align="center">
<img width="200" alt="オートモード画面：ReadySet" src="https://github.com/user-attachments/assets/b6cc84c0-597f-4b15-bbd8-a9ac42e3bc4b" />
</p>

ReadySetが表示されている間にベイをランチャーから外すと、電動ランチャー駆動（モーターの回転開始）がキャンセルされます。このキャンセル可能な時間を猶予時間（レイテンシ, latency）と呼び、デフォルトでは1.3秒間で設定されています。猶予時間の値はマニュアル・設定モードで変更することができます（詳細はマニュアル・設定モードを参照）。

<p align="center">
<img width="200" alt="オートモード画面：射出キャンセル" src="https://github.com/user-attachments/assets/e0c6d7b2-e969-4cf8-bf2d-ac70e578dcc2" />
</p>

ベイを外さないでおくとカウントダウンが始まりますので、号令にあわせて自分のシュートを行ってください。電動ランチャーは設定にしたがってベイの射出を行います。ディスプレイには自分のシュートパワーが表示されます。画面の見方は、前述の電動ランチャー無効状態でのシュートパワー測定と同じです（エラー表示も同様）。

<p align="center">
<img width="200" alt="オートモード画面：結果" src="https://github.com/user-attachments/assets/f488a416-c18b-4554-ac6b-dc4b3979d24b"/>
</p>

### 2-2-5. 待機画面の切替

押しボタンを押すと、待機画面の表示を切り替えることができます。以下の順番で切り替わります。

+ シュート統計
+ シュートパワー分布
+ デバイス情報・パラメータ

#### シュート統計の表示

オートモードで標準で表示されるのがシュート統計です。シュートパワーの統計情報が表示されます。

<p align="center">
<img width="200" alt="オートモード画面：統計表示" src="https://github.com/user-attachments/assets/7b7ecd12-14d1-4af3-98d0-f1df88e3c644" />
</p>

画面の意味は以下の通りです。

- `LAST` 最新SP値 # 合計シュート数
- `MEAN` 平均SP値 ± 標準偏差
- `RANGE` 最大SP値 ＼ 最小SP値

#### シュートパワー分布の表示

横軸をシュートパワー、縦軸を頻度（回数）に取ってシュートパワーの分布を表示します。k（キロ）はx1000です。

<p align="center">
<img width="200" alt="オートモード画面：ヒストグラム表示" src="https://github.com/user-attachments/assets/1ff227ab-a4ad-4d1f-b08f-b30546b59519" />
</p>

#### デバイス情報・パラメータの表示

デバイス情報と変更可能なパラメータを表示します。

<p align="center">
<img width="200" alt="オートモード画面：パラメータ表示" src="https://github.com/user-attachments/assets/1b1d82dd-105a-426c-abb8-34f1a010c9a5" />
</p>

画面の意味は以下の通りです。

- `VER` デバイスのバージョン番号
- `SP` メインで表示されるシュートパワー
  - `Bey Battle Pass` ベイバトルパスに記録されたシュートパワー値（アプリで読み込まれるのはこの値）をメインで表示する
  - `Estimated` 異常値を除外した推定シュートパワー値をメインで表示する
- 電動ランチャーの設定
  - `ELR1` 電動ランチャー1の設定
    - `A`もしくは`-` オートモードで用いる電動ランチャーに指定されているか否か
    - `M`もしくは`-` マニュアルモードで用いる電動ランチャーに指定されているか否か
    - `R`もしくは`L` 回転方向（`R`が右回転、`L`が左回転）
    - シュートパワー
    - モーターの最大回転数
  - `LTN` オートモードにおける電動ランチャー駆動（モーターの回転開始）のキャンセル可能な時間を猶予時間。ミリ秒単位
  - `DEL` オートモードにおけるカウントダウン最後の "Shoot!" の言い始めとモーター停止（ベイ射出）の間隔（射出遅延）。ミリ秒単位

## 2-3. マニュアル・設定モード (M)　SP計測器の場合は「読み出しモード」

### 2-3-1. 概要

マニュアル・設定モードは、電動ランチャーの設定を行ったり、電動ランチャー単体で遊ぶモード（手動での操作）です。
SP計測器の場合は「読み出しモード」としています。

**機能**
- 電動ランチャーの設定
  - シュートパワー
  - 回転方向 L/R
  - 有効・無効
- オートモードの設定
  - ベイ射出決定猶予時間（Readysetと3の合図の間の時間）
    - この時間内であれば、射出キャンセルが可能
  - 射出遅延時間（Shoot!の合図からの遅延時間）
  - 使用する電動ランチャー
- シュートパワー統計の読み出し

マニュアル・設定モードでは、ATLASとBluetooth接続するため**PC/タブレット/スマートフォン**が必須となります。

### 2-3-2. デバイス側

マニュアルモードのディスプレイ表示は以下の通りです。
押しボタンを押すことで画面が切り替わります（これ以外にもこのページのURL画面や、クライアントURL画面が表示されます）。
詳細はオートモード2-2-5節を参照してください。

<p align="center">
<img width="200" alt="マニュアル画面：統計" src="https://github.com/user-attachments/assets/a20296cd-9d08-433c-a186-fa0e8650bad8" />
<img width="200" alt="マニュアル画面：ヒストグラム" src="https://github.com/user-attachments/assets/e2552e46-27d6-407d-ba68-c2932a6bb851" />
<img width="200" alt="マニュアル画面：パラメータ" src="https://github.com/user-attachments/assets/32e6cea9-b65b-4791-a74f-44d75f16dfe3" />
</p>

ヘッダ領域の詳細は以下の通りです。

- `M` マニュアル/設定モードのアイコン
- `クライアントアイコン` PCやスマホなどのクライアントが接続されている場合に付くアイコン
- `EL1` 電動ランチャー1がマニュアル/設定モードで有効にされている場合に付くアイコン
- `EL2` 電動ランチャー2がマニュアル/設定モードで有効にされている場合に付くアイコン

SP計測器の場合は、`EL1`と`EL2`は表示されません。
  
### 2-3-3. クライアント側

ここでは、ATLASと通信するPC、スマホ等で動作させるクライアントソフトウェアについて記述します。
クライアントソフトウェアはWebアプリとして実装してあります。
Web Bluetooth APIがChromeでしか動かないため、Chromeをお使いください。
iPhoneの場合は、BluefyというブラウザでBluetoothが使えるようです。

ATLASとの通信アプリは以下のULRにアクセスすると使えます。

https://shark-minister.github.io/atlas_bey/

#### 接続

ATLASをマニュアル/設定モードに切り替えた状態で、〔接続する〕ボタンをクリックしてください。

<p align="center">
<img width="400" alt="connection" src="https://github.com/user-attachments/assets/3afbe3c1-41e9-49c1-9252-80afb8db29af" />
</p>

付近のATLASを検知したら、デバイスのリストに ATLAS_AUTO_LAUNCHER（Arduionの場合もあり）が表示されます。

<p align="center">
<img width="340" alt="接続" src="https://github.com/user-attachments/assets/503f0562-9e80-4ec7-aee9-709a2736eb2e" />
</p>

ATLAS_AUTO_LAUNCHER（or Arduiono）を選んで〔ペア設定〕をクリックするとATLASが接続されます。接続されると自動的にATLASのパラメータが読み込まれます。
以下の設定画面（後述）は以下の通りになります。

<p align="center">
<img width="340" alt="Webクライアント画面2" src="https://github.com/user-attachments/assets/d438f183-53c3-4e5a-945d-c80943a5ca62" />
</p>

#### 切断

〔接続する〕ボタンをクリックすれば、ATLASとの接続を切断できます。

#### SP計測モードに切替（SP計測器で、かつスイッチレスの場合のみ表示）

「Aモードに切替」ボタンは、SP計測器でスイッチレス型かタクトスイッチ型の場合のみ有効です。
クリックすると、SP計測モードに切り替わります。
（このボタンを使用せずとも、SP計測器を再起動すればSP計測モードで立ち上がります）

#### マニュアル制御（電動ランチャー制御として使う場合のみ表示）

〔ベイを射出する〕ボタンを押すとモータにセットされたベイが射出されます。

#### 設定（一部共通）

- 回転方向: モーターの回転方向を設定します。Rが右回転、Lが左回転です。
- シュートパワー: モーターの回転数を設定します。値は100の倍数である必要があります。
- 猶予時間: 猶予時間をミリ秒単位で設定します。値は10の倍数である必要があります。
- 遅延時間: 遅延時間をミリ秒単位で設定します。値は2の倍数である必要があります。
- BBPの記録値をメインにする:
  - 有効: BBPの記録SP値を大きく表示します（ワインダーランチャーではこちらがオススメ）
  - 無効: 異常値を除外した見込みシュートパワー値を大きく表示します（ストリングランチャーではこちらがオススメ）
- 〔読み出す〕ボタン: ATLASで動いているパラメータを読み込みます。
- 〔書き込む〕ボタン: ATLASにパラメータを送ります。

#### シュート情報

ATLASに保存されているシュートパワー統計情報を読み込んで、グラフ表示を行います。

- 〔読み出す〕ボタン: シュートパワー統計情報を読み込んで、グラフ表示を行う。
- 〔初期化〕ボタン: ATLASに保存されているシュート統計情報をクリアする。

<p align="center">
<img width="340" alt="シュート統計" src="https://github.com/user-attachments/assets/8f82318d-b840-4a8b-8af7-e0676e78d08a" />
</p>

<p align="center">
<img width="340" alt="シュート統計グラフ" src="https://github.com/user-attachments/assets/f001e7fe-d015-423a-80fb-58371c006cdc" />
</p>


# 3. ソフトウェアの書き込み（更新も含む）

ここでは、マイコンで動作するソフトウェアの書き込み（更新）について記述します。

SP計測器をお持ちの方で、ソフトウェアの更新をされる場合は、3-1節～3-5節をご覧下さい。
具体的には、以下の節を読んで頂ければソフトウェアの更新が可能です。

- 3-1節: 書き込みソフトウェアの準備
- 3-2節: ソースコードのダウンロード（Gitを使われる方を除き、3-2-2節の方）
- 3-3節: ソースコードのオープン
- 3-4-1節: setting.hhの表示
- 3-4-8節: SP計測器での設定について（setting.hhの書き換え）
- 3-5節: プログラムのビルドと書き込み

## 3-1. 書き込みソフトウェアの準備（ビルド環境）

### 3-1-1. Arduino IDEのインストール

マイコンへのソフトウェアの書き込みは、Arduino IDEというアプリケーションを用います。
Arduino IDEをインストールしていない場合は、下記URLから自分のプラットフォーム（OS）にあったものをダウンロードしてインストールしてください。

Arduino IDEの公式ダウンロードページ：
https://www.arduino.cc/en/software/

<p align="center">
<img width="701" height="349" alt="Arduino IDE download" src="https://github.com/user-attachments/assets/cd8efcfc-9b93-45bb-880c-25fb97276830" />
</p>

> [!NOTE]
> 本ソースコードは、Arduino IDEでビルドして書き込むことを想定しています。PlatformIOでのビルドは検証していません。

### 3-1-2. ボードマネージャのインストール

次の手順で、ボードマネージャから、**esp32 by Espressif Systems** をインストールします。

1. Arduino IDEを起動する
2. 左のアイコン列の上から2番目（図の①）をクリックする
3. 検索窓に **esp32-c3** と入力する（図の②）
4. **esp32 by Espressif Systems** であることを確認して、「インストール」をクリックする（図の③）

<p align="center">
<img width="443" height="343" alt="board-manager" src="https://github.com/user-attachments/assets/008cc653-62ff-48d4-b246-2e0324643ad4" />
</p>

### 3-1-3. ライブラリのインストール

以下のライブラリが必要になります。

- **ArduinoBLE**
  - 必須
  - Bluetooth接続を行うためのライブラリ
- **Adafruit SH110X**
  - 1.3インチディスプレイSH1106を使う場合、必須
  - SP計測器の標準版をお持ちの方はこちら
- **Adafruit SSD1306**
  - 0.96インチディスプレイSSD1306を使う場合、必須
  - SP計測器のコンパクト版をお持ちの方はこちら
- **DFRobotDFPlayerMini**
  - オーディオプレイヤーのDFPlayerMiniを使う場合、必須
  - SP計測器の場合は不要

次の手順で、各ライブラリをインストールします。

1. 左のアイコン列の上から3番目（図の①）をクリックする
2. 検索窓にライブラリ名（上記のコピー＆ペーストが便利）を入力する（図の②）
3. 一番上に出てくるライブラリの「インストール」ボタンをクリックし、インストールを行う（図の③）
4. 必要なライブラリだけ、1-3を繰り返す

<p align="center">
<img width="304" height="344" alt="library" src="https://github.com/user-attachments/assets/e952eca0-3ef3-4d67-9004-4cddc26505ca" />
</p>

## 3-2. ソースコードのダウンロード

Gitを使ってソースコードを取得する場合は 5-2-1 節、そうでない場合は 5-2-2節 を参照してください。

### 3-2-1. Gitを使う場合

適切なディレクトリにて、以下のコマンドでダウンロードしてください。

```
$ git clone git@github.com:shark-minister/atlas_bey.git
```

### 3-2-2. Gitを使わない場合

このページ最上部で、**<> Code** をクリックして（図の①）、Download Zip を選んでください（図の②）。

<p align="center">
<img width="698" height="352" alt="download" src="https://github.com/user-attachments/assets/eb3d4c55-26cf-4cfd-b55d-dfa145b8670f" />
</p>

ダウンロードしたファイルを選択し（図の①）、「すべて展開」をクリックしてください（図の②）。

<p align="center">
<img width="654" alt="展開" src="https://github.com/user-attachments/assets/16824325-9e93-4542-855e-4c21f61c6e3c" />
</p>

展開が終わると、atlas_bey-main といフォルダができます。

<p align="center">
<img width="600" alt="展開後" src="https://github.com/user-attachments/assets/e9de7b4c-a25a-4167-9bba-2231e31720e0" />
</p>

## 3-3. プロジェクトのオープン

Arduino IDEで 3-2節でダウンロードしたプロジェクトを開きます。

1. 「ファイル」メニューの「開く...」をクリックする（図の①）
2. 3-2節でダウンロードしたフォルダの中の **atlas.ino** を選択してください（図の②）。
   - Gitを使用した場合、以下のフォルダの中にあります
     - atlas_bey > atlas > atlas.ino
   - Gitを使用していない場合は、展開によってできたフォルダの中を見ます（図の下線部を参照）
     - atlas_bey-main > atlas_bey-main > atlas > atlas.ino

<p align="center">
<img width="723" alt="open" src="https://github.com/user-attachments/assets/e36dbc08-8380-42b2-ad35-4f7aeef6298c" />
</p>

あるいは、エクスプローラ上で atlas.ino をダブルクリックしても、Arduino IDEで開くことができます。

<p align="center">
<img width="711" alt="arduino.ino" src="https://github.com/user-attachments/assets/09f48ae9-18de-4df5-bb68-6daf889ea003" />
</p>

プロジェクトを開くと以下の画面になります。

<p align="center">
<img width="897" alt="window" src="https://github.com/user-attachments/assets/ebdbd209-9c3a-40b6-b0eb-0bc09d62626c" />
</p>

## 3-4. ソフトウェア設定値の変更

ソフトウェア設定値の変更は、`setting.hh`で行えます。

### 3-4-1. setting.hh の表示

setting.hhのソースコードは、**setting.hh** のタブを選択すると表示されます。
setting.hhのタブは、この順番にあるとは限りません。

<p align="center">
<img width="348" alt="setting.hh" src="https://github.com/user-attachments/assets/82001963-01b6-4dd4-9d67-c7b2df68d82d" />
</p>

必要に応じて書き換えてください。
**SP計測器**のプログラム更新を行う場合は3-4-2節から3-4-7節を読む必要はありませんので飛ばして、「**3-4-8. SP計測器での設定について**」に進んでください。

### 3-4-2. ハードウェアの実装（共通）

#### 機器の利用形態（共通）

`ATLAS_FORMAT`に以下の定数のいずれかを設定する。

- `ATLAS_FULL_SPEC` 電動ランチャー制御としても使う、フルスペックATLASとしてビルドする
- `ATLAS_LITE_SP_ONLY` SP計測器のみで使う、ATLAS liteとしてビルドする

#### モード切替スイッチの種類（共通）

`SWITCH_TYPE`に以下の定数のいずれかを設定する。

- `SW_NONE` スイッチなし（物理スイッチが無効になる）
- `SW_SLIDE` スライドスイッチを使う（ver.1.2.xまで）
- `SW_TACT` タクトスイッチを使う（ver.1.3.0以降）

#### ディスプレイの種類（共通）

`DISPLAY_MODEL`に以下の定数のいずれかを設定する。

- `ADAFRUIT_SH1106G` SH1106G (1.3インチ)
- `ADAFRUIT_SSD1306` SSD1306 (0.96インチ)

#### モーター関連（電動ランチャー制御として使う場合のみ）

- `NUM_MOTORS`
  - 使用するモーターの数
  - デフォルト値: 1
- `USE_DUMMY_MOTOR`
  - モーターをダミーモードにする。デバッグ用。モータードライバーに接続せず、Serial通信でデバッグ文を出力するのみとなる。
  - デフォルト値: 0

### 3-4-3. ピン番号設定（共通）

#### スイッチ関連

- `MODE_SW_IN` 切替スイッチの中央端子と繋ぐピン番号
- `MODE_SW_OUT` 切替スイッチのオートモード側端子と繋ぐピン番号

<table>
  <tr>
    <th></th>
    <th colspan="2">フルスペック機ATLAS</th>
    <th colspan="2">SP計測機ATLAS lite</th>
  </tr>
  <tr>
    <th>スイッチ</th>
    <th>タクト</th>
    <th>スライド</th>
    <th>タクト</th>
    <th>スライド</th>
  </tr>
  <tr>
    <th>MODE_SW_IN</th>
    <td>8</td>
    <td>8</td>
    <td>5</td>
    <td>5</td>
  </tr>
  <tr>
    <th>MODE_SW_OUT</th>
    <td>不要</td>
    <td>5</td>
    <td>不要</td>
    <td>21</td>
  </tr>
</table>

#### モーター関連

- `L_PWM_1` 
   - モータードライバー1のL_PWMをつなぐGPIOピン番号
   - デフォルト値: 2
   - 左回転用PWM
- `R_PWM_1` 
   - モータードライバー1のR_PWMをつなぐGPIOピン番号
   - デフォルト値: 3
   - 右回転用PWM
- `LR_EN_1` 
   - モータードライバー1のL_ENとR_ENをつなぐGPIOピン番号
   - デフォルト値: 4
- `L_PWM_2` 
   - モータードライバー2のL_PWMをつなぐGPIOピン番号
   - デフォルト値: 11
   - 左回転用PWM
- `R_PWM_2` 
   - モータードライバー2のR_PWMをつなぐGPIOピン番号
   - デフォルト値: 10
   - 右回転用PWM
- `LR_EN_2` 
   - モータードライバー2のL_ENとR_ENをつなぐGPIOピン番号
   - デフォルト値: 4（モータードライバー1と共通）

> [!CAUTION]
> モーター2台体制は未検証です。

#### オーディオ関連

- `AUDIO_RX`
   - シリアル通信RX
   - デフォルト値: 20
- `AUDIO_TX`
   - シリアル通信TX
   - デフォルト値: 21

### 3-4-4. ディスプレイパラメータ（共通）

ディスプレイにSSD1306を使用する場合、`DISPLAY_MODEL`を`ADAFRUIT_SSD1306`に変更してください。

- `SCREEN_WIDTH`
  - スクリーンの幅
  - デフォルト値: 128
- `SCREEN_HEIGHT`
  - スクリーンのt高さ
  - デフォルト値: 64
- `SCREEN_ADDR`
  - I2Cアドレス
  - デフォルト値: 0x3C
- `DISPLYA_IS_SPI`
  - ディスプレイがSPI接続のときは1，I2Cのときは0
- `DISPLAY_DRIVER`
  - ディスプレイのモデル。以下のいずれかの値をとる
  - ADAFRUIT_SSD1306: SSD1306を使う場合
  - ADAFRUIT_SH1106G: SH1106Gを使う場合

### 3-4-5. モーター設定（電動ランチャー制御として使う場合のみ）

- `MOTOR1_MAX_RPM`
  - モーター1の最大回転数
  - デフォルト値: 24,900
- `MOTOR2_MAX_RPM`
  - モーター2の最大回転数
  - デフォルト値: 24,900

> [!CAUTION]
> モーターは、タミヤの<ins>OP.1393 380 スポーツチューンモーター</ins>か、<ins>OP.68 RS-540 スポーツチューンモーター</ins>を想定しています。
> それ以外のモーターでは動作を検証していません。

### 3-4-6. 音声設定（電動ランチャー制御として使う場合のみ）

- `AUDIO_COUNTDOWN`
  - カウントダウン音声の番号
  — 3, 2, 1, Go, shootの間隔は1秒とする
  - デフォルト値: 1
- `AUDIO_SE_ACK`
  - 成功の効果音の番号
  - デフォルト値: 2
- `AUDIO_SE_CANCEL`
  - キャンセルの効果音の番号
  - デフォルト値: 3 
- `AUDIO_SE_ERROR`
  - エラーの効果音の番号
  - デフォルト値: 4 
- `DEFAULT_VOLUME`
  - デフォルト音量
  - デフォルト値: 20

### 3-4-7. 動作パラメータ設定（電動ランチャー制御として使う場合のみ）

- BLE通信でPC・スマホから変更可能なパラメータのデフォルト値
  - `DEFAULT_LAUNCHER_SP`
    - 電動ランチャーのSPデフォルト値
    - 電動ランチャーでベイを射出するときのシュートパワーSPのデフォルト値を指定する。SPの内部パラメータは、外部からBLE通信で変更できる。
    - デフォルト値: 10,000
  - `DEFAULT_DELAY`
    - 射出遅延時間のデフォルト値 [ms]
    - モーター停止（ベイ射出）の間隔（射出遅延）のデフォルト値をミリ秒で指定する。遅延時間の内部パラメータは、外部からBLE通信で変更できる。
    - デフォルト値: 0
  - `DEFAULT_LATENCY`
    - 猶予時間のデフォルト値 [ms]
    - オートモードでのカウントダウン最初の "ReadySet" と "3" までの間隔のデフォルト値をミリ秒で指定する。この猶予時間内にベイをランチャーから外すとカウントダウンとモーター駆動開始がキャンセルされる。猶予時間の内部パラメータは、外部からBLE通信で変更できる。
    - デフォルト値: 1,300
- ソフトウェアリミット
  - `LAUNCHER_SP_UPPER_LIMIT`
    - 電動ランチャーのSP上限値（ソフトウェアリミット）
    - 電動ランチャーでベイを射出するときのシュートパワーSPの上限値を指定する。モーターの最高回転数以下にすることが望ましい。この値は外部から変更できない。
    - デフォルト値: 24,900
  - `LAUNCHER_SP_LOWER_LIMIT`
    - 電動ランチャーのSP下限値（ソフトウェアリミット）
    - 電動ランチャーでベイを射出するときのシュートパワーSPの下限値を指定する。この値は外部から変更できない。
    - デフォルト値: 3,000
  - `DELAY_UPPER_LIMIT`
    - 射出遅延時間の上限値（ソフトウェアリミット） [ms]
    - オートモードでのカウントダウン最後の "Shoot!" の言い始めとモーター停止（ベイ射出）の間隔（射出遅延）の上限値をミリ秒で指定する。この値は外部から変更できない。
    - デフォルト値: 500
  - `LATENCY_LOWER_LIMIT`
    - 猶予時間の下限値（ソフトウェアリミット） [ms]
    - オートモードでのカウントダウン最初の "ReadySet" と "3" までの間隔の下限値をミリ秒で指定する。この値は外部から変更できない。
    - デフォルト値: 500
- その他
  - `COUNTDOWN_INTERVAL`
    - カウントダウンコールの間隔 [ms]
    - カウントダウンコールは、"3, 2, 1, Go, Shoot!" であり、そのコール間隔をミリ秒で指定する。この値は外部から変更できない。
    - デフォルト値: 1,000
  - `MOTOR_PREPARATORY_TIME`
    - モーター安定の猶予時間 [ms]
    - マニュアル射出の際、モーターが最大回転数になっても回転が安定するまでに猶予時間をとっておいた方が良いので、それを考慮した値をミリ秒で指定する。この値は外部から変更できない。マニュアルモード時のみ使用される。
    - デフォルト値: 2,000
  - `SYNC_ADJ_TIME`
    - オートモード射出・音声・表示の同期調整時間 [ms]
    - 音声タスクとモーター駆動タスク・表示タスクの同期をとるための調整時間。この値は外部から変更できない。
    - デフォルト値: 300

### 3-4-8. SP計測器での設定について

SP計測器用のプログラムを書き込みたい場合は、setting.hh に以下の書き換えを行ってください。

<p align="center">
<img width="514" alt="SP_MEAS" src="https://github.com/user-attachments/assets/435fec6d-851e-4350-b96b-cc457e0ef085" />
</p>

**①利用形態（21行目）**
- **ATLAS_FORMAT** の値を **ATLAS_LITE_SP_ONLY** に設定する

**②スイッチの種類（27行目）**
- **SWITCH_TYPE** の値を 機器のスイッチ形態に合わせて設定する
  - **SW_NONE** - スイッチがない
  - **SW_SLIDE** - スライドスイッチ
  - **SW_TACT** - 押しボタンスイッチ

**③ディスプレイタイプ（32行目）**
- **DISPLAY_DRIVER** の値を、機器のディスプレイタイプに合わせて設定する
  - **ADAFRUITE_SH1106G** - 1.3インチディスプレイ（SP計測器の標準版）
  - **ADAFRUITE_SSD1306** - 0.96インチディスプレイ（SP計測器のコンパクト版）

## 3-5. プログラムの書き込み

使用デバイスにあった設定ができたら、プログラムをビルドして書き込みを行います。
まず、デバイスをUSBケーブルでつなぎ、ツールバーのデバイスリストの「他のボードとポートを選択」をクリック（画像オレンジ色で囲った部分）します。

<p align="center">
<img width="424" height="188" alt="board" src="https://github.com/user-attachments/assets/5a0c3b27-57a0-47c6-beb3-d300ea953c88" />
</p>

以下の手順で、ボードとポートを選択してください。

1. ボードの検索窓に **xiao** と入力する（図の①）
2. **XIAO_ESP32C3** を選択する（図の②）
3. ポートは、出てくるUSBポートのものを選ぶ（図の③）
4. OKを押す（図の④）

<p align="center">
<img width="554" height="389" alt="board選択" src="https://github.com/user-attachments/assets/11793193-18f3-4fe3-8974-156dfc7cb0c5" />
</p>

正しく選択できていれば、以下の図のようになります。

<p align="center">
<img width="345" height="126" alt="board1" src="https://github.com/user-attachments/assets/2f917e44-fa17-4f08-b757-80238d30755f" />
</p>

→ボタンを押すとビルドとマイコンへの書き込みが行われます。

<p align="center">
<img width="292" alt="board1" src="https://github.com/user-attachments/assets/b5688cfd-962f-4a72-84d2-cb37284bfe16" />
</p>

書き込みが終わると以下のような出力がなされ、デバイスも再起動します。

<p align="center">
<img width="734" alt="output1" src="https://github.com/user-attachments/assets/be7a7660-21f9-453b-be90-94c67d63af10" />
</p>

# 4. ハードウェア構成

## 4-1. 構成部品

### 4-1-1. 制御系

制御系の部品は以下の通りです。

- **マイコン**
  - Seeed Studio XIAO ESP32-C3
  - Bluetooh Low Energy対応のコンパクトなマイコン
  - 秋月電子: https://akizukidenshi.com/catalog/g/g117454/
  - スイッチサイエンス: https://www.switch-science.com/products/8348
  - 電子工作ステーション: https://electronicwork.shop/items/64fb421a4adc32002be4fa50
- **オーディオモジュール**
  - DFPlayer Mini
  - MicroSDカードに保存した音声ファイルを再生できるモジュール。互換品もあるが、Arduino以外で動かないという報告もあるので、正規品が無難
  - 秋月電子: https://akizukidenshi.com/catalog/g/g112544/
  - スイッチサイエンス: https://www.switch-science.com/products/4291
  - 電子工作ステーション: https://electronicwork.shop/items/639fc4a36b87c328989fcec5
  - 音声ファイルを格納しておく microSD カードが別途必要です。
- **ディスプレイ**
  - SH1106 (1.3" OLED)
  - Adafruit製のドライバーで制御できる 128x64 のディスプレイ
  - 単色ですが、色はいろいろあるようなのでお好みで
  - Amazon.co.jp: https://www.amazon.co.jp/dp/B0D9BKWX4H/ 他
  - SSD1306 (0.96" OLED) も使用可能（一部ソースコードの書き換えが必要。5-2-2節参照）
- **モータードライバー**
  - 12A対応のPWMモータードライバー
  - DAOKAI BTS7960など
  - Amazon.co.jp: https://www.amazon.co.jp/dp/B0B82GXBYF/ 他
  - モーターを2台接続する場合は、モータードライバーも2つ必要
- **スピーカー**
  - Wattsの500円スピーカーを分解して使いました。なんでも良いと思います（ホント？）。
- **スイッチ**
  - タクトスイッチ
- **ジャンプワイヤー等のケーブル**

<p align="center">
<img width="700" alt="ctrl_devices" src="https://github.com/user-attachments/assets/5bfff615-dbf2-4bd0-9fef-270805743ddb" />
</p>

### 4-1-2. 駆動系

駆動系の部品は以下の通りです。

1. **モーター**
   - タミヤ OP.1393 380 スポーツチューンモーター
   - シャフト径2.3→3.175mmのピニオン変換アダプタが付属
   - Amazon.co.jp: https://www.amazon.co.jp/dp/B008GY168C/
2. **ピニオン変換アダプタ**
   - シャフト径を3.175mm→5mmに変換するアダプタ
   - Amazon.co.jp: https://www.amazon.co.jp/dp/B0CMPS81FP/
3. **カップラ**
   - 5mmφのシャフトをM4に変換
   - Amazon.co.jp: https://www.amazon.co.jp/dp/B08NP39Y31/
4. **ベイブレードXランチャーの爪部分**
5. **M4ネジ**
   - ランチャーの爪をカップラに固定するためのネジ
   - 10mm未満がよい
   - スペーサーとしてナットが必要になる場合も
6. **電源**
   - 7.2V, 10Aを供給できる安定化電源
   - DROKスイッチング電源 0-12V / 20A など
   - Amazon.co.jp: https://www.amazon.co.jp/dp/B0B74NT7PR/
7. **モーター用ケーブル**
   - 2芯1.25sq VCT-F ケーブル
   - 電源・モータードライバー・モーター間の配線に使用する
   - ホームセンターで、切り売り 24円@10cm程度、5m梱包で750円程度
8. **電源ケーブル**
   - 片側がプラグになっている2芯1.25sqのケーブル
   - Amazon.co.jp: https://www.amazon.co.jp/dp/B00FIXSEO8/

1-5のパーツは下記のように組み立てます。

<p align="center">
<img width="600" alt="motor-assembly" src="https://github.com/user-attachments/assets/b50a89ba-130a-4447-a269-93e7dd933ac2" />
</p>

完成すると以下の図のようになります。左回転用は別途用意するか、ランチャーの爪を付け替えます。

<p align="center">
<img width="360" alt="motor-assembly" src="https://github.com/user-attachments/assets/130de011-55aa-4063-b0c9-21f8ef8e90d3" />
</p>

> [!NOTE]
> 3Dプリンタが使える人は、ランチャーの爪と2.3mmシャフトを繋ぐアダプタを自作した方が早いかも知れません。

### 4-1-3. その他

- **マジックアーム**
  - モーターを固定するためのジグ。両端にクランプが必要
  - モーターの間にゲルテープ（百均で売っている）を巻き、クランプで挟む

### 4-1-4. 音声データ

音声データは、現状で4つです。

- 01: カウントダウン音声（3, 2, 1, Go, shoot）
- 02: 成功の効果音
- 03: キャンセルの効果音
- 04: エラーの効果音

microSDカードに 01, 02, 03, 04のフォルダを作成し、それぞれのフォルダの中に001.mp3という名でファイルを保存してください。
カウントダウン音声の号令間隔は1秒です。

## 4-2. 配線

下の図の通り配線しました。

<p align="center">
<img width="700" alt="system-wiring" src="https://github.com/user-attachments/assets/b5dc64ba-1503-43b5-b2b0-5c735af7fa0d" />
</p>

SP計測器専用（ATLAS lite）での配線は以下の通りです。

<p align="center">
<img width="600" alt="wiring_compact" src="https://github.com/user-attachments/assets/01a6823d-130f-4b84-ba1e-ef3fa7366c24" />
</p>

### 4-2-1. BTS7960

モータードライバー BTS7960 の配線は以下の通りです。

> [!IMPORTANT]
> モータードライバーと電源・モーター間のケーブルは大きい電流が流れるので、1.25sq以上の太さのケーブルを使ってください。
> ホームセンターで販売している2芯1.25sqのケーブルの両端の被覆を剥いて、2本の線（白と黒）をそれぞれ＋と－で用いれば動力系の配線がすっきりします。

<table>
  <tr>
    <th>BTS7960</th>
    <th>ESP32-C3</th>
    <th>モーター</th>
    <th>電源 (7.2V)</th>
  </tr>
  <tr>
    <td>LPWM</td>
    <td>GPIO2</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>RPWM</td>
    <td>GPIO3</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>L_EN</td>
    <td>GPIO4</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>R_EN</td>
    <td>GPIO4</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>M＋</td>
    <td></td>
    <td>＋</td>
    <td></td>
  </tr>
  <tr>
    <td>M－</td>
    <td></td>
    <td>－</td>
    <td></td>
  </tr>
  <tr>
    <td>＋</td>
    <td></td>
    <td></td>
    <td>＋</td>
  </tr>
  <tr>
    <td>－</td>
    <td></td>
    <td></td>
    <td>－</td>
  </tr>
</table>

### 4-2-2. SH1106 / SSD1306 (I2C)

ディスプレイ SH1106 / SSD1306 (I2C) の配線は以下の通りです。

<table>
  <tr>
    <th>SH1106 / SSD1306</th>
    <th>ESP32-C3</th>
  </tr>
  <tr>
    <td>VCC</td>
    <td>3V3 (3.3V)</td>
  </tr>
  <tr>
    <td>GND</td>
    <td>GND</td>
  </tr>
  <tr>
    <td>SCL</td>
    <td>SCL</td>
  </tr>
  <tr>
    <td>SDA</td>
    <td>SDA</td>
  </tr>
</table>

### 4-2-3. DFPlayer Mini

音楽プレーヤー DFPlayer Mini の配線は以下の通りです。

<table>
  <tr>
    <th>DFPlayer Mini</th>
    <th>ESP32-C3</th>
    <th>スピーカー</th>
  </tr>
  <tr>
    <td>VCC</td>
    <td>3V3 (3.3V)</td>
    <td></td>
  </tr>
  <tr>
    <td>GND</td>
    <td>GND</td>
    <td></td>
  </tr>
  <tr>
    <td>RX</td>
    <td>TX</td>
    <td></td>
  </tr>
  <tr>
    <td>TX</td>
    <td>RX</td>
    <td></td>
  </tr>
  <tr>
    <td>SPK_1</td>
    <td></td>
    <td>＋</td>
  </tr>
  <tr>
    <td>SPK_2</td>
    <td></td>
    <td>－</td>
  </tr>
</table>

### 4-2-4. スイッチ

スイッチの配線は以下の通りです。

<table>
  <tr>
    <th>スイッチ</th>
    <th>ESP32-C3</th>
  </tr>
  <tr>
    <td>ピン1</td>
    <td>GPIO5 (INPUT_PULLUP)</td>
  </tr>
  <tr>
    <td>ピン2</td>
    <td>GND</td>
  </tr>
</table>

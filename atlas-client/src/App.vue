<script setup lang="ts">
import { useBLE } from "../classes/atlas"
import AtlasButton from "../components/AtlasButton.vue"
import ToggleButton from "../components/ToggleButton.vue"
import AtlasPanel from "../components/AtlasPanel.vue"
import SpChart from "../components/SpChart.vue"
import { Histogram } from "../classes/histogram"
import { ref } from "vue"

const {
  // データ
  devInfo,
  params,
  stats,
  histogram,
  // フラグ
  isConnected,
  isGattBusy,
  // 操作
  requestDevice,
  connect,
  disconnect,
  readDeviceInfo,
  readParams,
  writeParams,
  readStatistics,
  clearStatistics,
  launchManually,
  switchToAutoMode
} = useBLE()

// デバッグ用
// devInfo.value.format = 0
// devInfo.value.switchType = 2
// devInfo.value.numElrs = 2

const testHist = ref(new Histogram)
testHist.value.label = [
  "1000", "2000", "3000", "4000", "5000"
]
testHist.value.data = [
  10, 20, 40, 30, 15
]


async function confirmClearStatistics() {
  const msg = "デバイス内のデータを初期化しますか？<br/>Are you share to clear data in the device."
  if (confirm(msg)) {
    await clearStatistics()
  }
}

async function startAtlas() {
  await requestDevice()
  await connect()
  await readDeviceInfo()
  await readParams()
}
</script>

<template>
  <header>
    ATLAS Client
    <div>&minus; AuTomatic LAuncher System &minus;</div>
  </header>
  <div class="layout">

    <!-- 制御 -->
    <atlas-panel
      title-jpn="制御"
      title-eng="control"
    >
      <template #content>
        <div class="ctrl-buttons">
          <atlas-button
            label-jpn="接続"
            label-eng="connect"
            :disabled="isConnected || isGattBusy"
            @click="startAtlas()"
          />
          <atlas-button
            label-jpn="切断"
            label-eng="disconnect"
            :disabled="!isConnected || isGattBusy"
            @click="disconnect()"
          />
          <atlas-button
            label-jpn="Aモードに切替"
            label-eng="switch to A mode"
            :disabled="
              !isConnected ||
              isGattBusy ||
              !devInfo.useSoftwareSwitch()
            "
            @click="switchToAutoMode()"
          />
          <atlas-button
            label-jpn="ベイ射出"
            label-eng="launch beyblade"
            :disabled="
              !isConnected ||
              isGattBusy ||
              !devInfo.isElrController()
            "
            @click="launchManually()"
          />
        </div>
      </template>
    </atlas-panel>

    <!-- 設定 -->
    <atlas-panel
      title-jpn="パラメータ"
      title-eng="parameters"
    >
      <template #content>

        <div class="params-table">
          <!-- 電動ランチャー独自の設定 -->
          <template v-if="devInfo.isElrController()">
            <div class="title">
            </div>
            <div class="elr1 heading">
              ELR 1
            </div>
            <div class="elr2 heading">
              ELR 2
            </div>

            <div class="title">
              回転方向
              <div class="eng">Rotation</div>
            </div>
            <div class="elr1">
              <toggle-button
                type="LR"
                :model="params.elr1.isRotRight"
                @click="params.elr1.isRotRight = $event"
              />
            </div>
            <div class="elr2">
              <toggle-button
                type="LR"
                :model="params.elr2.isRotRight"
                @click="params.elr2.isRotRight = $event"
              />
            </div>

            <div class="title">
              シュートパワー
              <div class="eng">Shoot power</div>
            </div>
            <div class="elr1">
              <input
                type="number"
                class="number"
                id="elr1-sp"
                min="3000"
                max="24000"
                step="100"
                v-model.number="params.elr1.sp"
              />
              <div class="note">※100の倍数</div>
            </div>
            <div class="elr2">
              <input
                type="number"
                class="number"
                id="elr2-sp"
                min="3000"
                max="24000"
                step="100"
                v-model.number="params.elr2.sp"
              />
              <div class="note">※100の倍数</div>
            </div>

            <div class="title">
              マニュアルモード
              <div class="eng">Manual mode</div>
            </div>
            <div class="elr1">
              <toggle-button
                :model="params.elr1.enabledManual"
                @click="params.elr1.enabledManual = $event"
              />
            </div>
            <div class="elr2">
              <toggle-button
                :model="params.elr2.enabledManual"
                @click="params.elr2.enabledManual = $event"
              />
            </div>

            <div class="title">
              オートモード
              <div class="eng">Auto mode</div>
            </div>
            <div class="common">
              <toggle-button
                type="12"
                :model="params.elr2Auto"
                @click="params.elr2Auto = $event"
              />
            </div>

            <div class="title">
              猶予時間 [ms]
              <div class="eng">Latency</div>
            </div>
            <div class="common">
              <input
                type="number"
                class="number"
                id="latency"
                min="500"
                max="2560"
                step="10"
                v-model.number="params.latency"
              />
              <div class="note">※偶数</div>
            </div>

            <div class="title">
              遅延時間 [ms]
              <div class="eng">Delay</div>
            </div>
            <div class="common">
              <input
                type="number"
                class="number"
                id="delay"
                min="0"
                max="500"
                step="2"
                v-model.number="params.delay"
              />
              <div class="note">※偶数</div>
            </div>
          </template>

          <div class="title">
            BBP採用SP値をメインに表示
            <div class="eng">BBP SP main</div>
          </div>
          <div class="common">
            <toggle-button
              :model="params.isBbpSpMain"
              @click="params.isBbpSpMain = $event"
            />
          </div>
        </div>

        <div class="buttons">
          <atlas-button
            label-jpn="読み出す"
            label-eng="read parameters"
            :disabled="!isConnected || isGattBusy"
            @click="readParams()"
          />
          <atlas-button
            label-jpn="書き込む"
            label-eng="write parameters"
            :disabled="!isConnected || isGattBusy"
            @click="writeParams()"
          />
        </div>
      </template>
    </atlas-panel>

    <!-- 統計データ -->
    <atlas-panel
      title-jpn="統計データ"
      title-eng="statistics data"
    >
      <template #content>

        <div class="stats-table">
          <div class="title">
            シュート数
            <div class="eng">Num. shoots</div>
          </div>
          <div class="value">
            <input
              type="number"
              class="sp-data"
              readonly="true"
              v-model="stats.total"
            />
          </div>

          <div class="title">
            最大SP
            <div class="eng">Max SP</div>
          </div>
          <div class="value">
            <input
              type="number"
              class="sp-data"
              readonly="true"
              v-model="stats.maxSp"
            />
          </div>

          <div class="title">
            最小SP
            <div class="eng">Min SP</div>
          </div>
          <div class="value">
            <input
              type="number"
              class="sp-data"
              readonly="true"
              v-model="stats.minSp"
            />
          </div>

          <div class="title">
            平均SP
            <div class="eng">Mean SP</div>
          </div>
          <div class="value">
            <input
              type="number"
              class="sp-data"
              readonly="true"
              v-model="stats.avgSp"
            />
          </div>

          <div class="title">
            標準偏差
            <div class="eng">Std. deviation</div>
          </div>
          <div class="value">
            <input
              type="number"
              class="sp-data"
              readonly="true"
              v-model="stats.stdSp"
            />
          </div>
        </div>

        <div class="buttons">
          <atlas-button
            label-jpn="読み出す"
            label-eng="read statistics"
            :disabled="!isConnected || isGattBusy"
            @click="readStatistics()"
          />
          <atlas-button
            label-jpn="初期化する"
            label-eng="clear statistics"
            :disabled="!isConnected || isGattBusy"
            @click="confirmClearStatistics()"
          />
        </div>

      </template>
    </atlas-panel>

    <!-- SPヒストグラム -->
    <atlas-panel
      title-jpn="SP分布"
      title-eng="SP distribution"
    >
      <template #content>
        <div class="chart-container">
          <sp-chart
            v-if="stats.total > 0"
            :hist="histogram"
          />
        </div>
      </template>
    </atlas-panel>
  </div>
  <footer>
    &copy;2025,2026 shark minister
  </footer>
</template>

<style lang="scss" scoped>

// スマートフォン
.layout {
  display: grid;
  grid-template-columns: 1fr;
}
// タブレット
@media (min-width: 800px) { 
  .layout {
    display: grid;
    grid-template-columns: 1fr 1fr;
  }
}
// PC
@media (min-width: 1200px) { 
  .layout {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
  }
}
// 大きいPC
@media (min-width: 1600px) { 
  .layout {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr 1fr;
  }
}

//-----------------------------------------------------------------------------

header {
  text-align: center;
  margin-left: 0.24rem;
  margin-top: 0;
  margin-bottom: 0.3rem;
  font-size: 1.3rem;
  font-weight: bold;
  div {
    margin-top: 0.3rem;
    font-size: 0.6rem;
    font-weight: normal;
  }
}

footer {
  margin-top: 0.5rem;
  text-align: center;
  font-size: 0.7rem;
}

.ctrl-buttons {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 1fr;
  grid-template-rows: 1fr;
  column-gap: 0.2rem;
}

.buttons {
  display: flex;
  justify-content: center;
  align-items: center;
  
  button {
    margin-right: 0.3rem;

    &:last-child {
      margin-right: 0;
    }
  }
}

//-----------------------------------------------------------------------------
// 設定表
//-----------------------------------------------------------------------------
.params-table {
  font-size: 0.9rem;
  text-align: center;
  margin-bottom: 1rem;
  // グリッド配置
  display: grid;
  grid-template-columns: 1fr 0.7fr 0.7fr;
  grid-template-rows: repeat(8, auto);
  column-gap: 1rem;
  row-gap: 0.5rem;

  .title {
    grid-column: 1;
    text-align: right;
    font-weight: bold;
    color: rgb(46, 46, 46);

    .eng {
      margin-top: 0.3rem;
      font-size: 0.75rem;
      font-weight: normal;
      color: rgb(114, 172, 184);
    }
  }
  
  .note {
    margin-top: 0.2rem;
    font-size: 0.65rem;
    text-align: right;
    color: rgb(114, 172, 184);
  }

  div {
    display: grid;
    align-content: center;
  }

  .elr1 {
    grid-column: 2;
  }
  .elr2 {
    grid-column: 3;
  }
  .common {
    grid-column: 2 / span 2;
  }
  .heading {
    font-weight: bold;
  }

  input {
    width: 100%;

    &:invalid {
      background-color: #ff8889;
    }

    &.number {
      box-sizing: border-box;
      text-align: right;
      font-size: 0.9rem;
      height: 1.7rem;
      border: 0;
      border-radius: 1ex;
      background-color: rgb(229, 247, 250);
    }
  }
}

//-----------------------------------------------------------------------------
// 統計データ表
//-----------------------------------------------------------------------------
.stats-table {
  font-size: 0.9rem;
  margin-bottom: 1rem;
  display: grid;
  grid-template-columns: auto auto;
  grid-template-rows: repeat(5, auto);
  column-gap: 1rem;
  row-gap: 0.5rem;

  .title {
    grid-column: 1;
    text-align: right;
    font-weight: bold;
    color: rgb(46, 46, 46);

    .eng {
      margin-top: 0.3rem;
      font-size: 0.75rem;
      font-weight: normal;
      color: rgb(114, 172, 184);
    }
  }

  .value {
    input {
      font-size: 1rem;
      font-weight: bold;
      width: 6rem;
      text-align: right;
      height: 2rem;
      border: 0;
      border-radius: 1ex;
      background-color: rgb(229, 247, 250);
    }
  }

  div {
    display: grid;
    align-content: center;
  }
}

.chart-container {
  height: 300px;
  width: 100%;
}
</style>

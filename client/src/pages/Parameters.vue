<script setup lang="ts">
import { useATLAS } from '@/classes/atlas'
import { useDisplay } from "vuetify"
import {
  mdiWrenchCog,
  mdiUpload,
  mdiDownload
} from '@mdi/js'
import ToggleButton from '@/components/ToggleButton.vue'

const {
  devInfo,
  params,
  isConnected,
  isGattBusy,
  readParams,
  writeParams,
} = useATLAS()

const { mobile } = useDisplay()

</script>

<template>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="parameters"
    :prepend-icon="mdiWrenchCog"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        パラメータ
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          パラメータの読み出し・書き込みを行います。
        </div>
        <div>
          Read / write parameters.
        </div>
      </div>

      <div class="two-button-layout">
        <v-btn
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiUpload"
          :disabled="!isConnected || isGattBusy"
          @click="readParams()"
        >
          <span>
            <div class="mb-1">
              読み出す
            </div>
            <small class="btn-label-eng">
              read
            </small>
          </span>
        </v-btn>

        <v-btn
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiDownload"
          :disabled="!isConnected || isGattBusy"
          @click="writeParams()"
        >
          <span>
            <div class="mb-1">
              書き込む
            </div>
            <small class="btn-label-eng">
              write
            </small>
          </span>
        </v-btn>
      </div>

      <div class="params-table">
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
          <template v-if="devInfo.isElrController()">
            <toggle-button
              type="LR"
              :model="params.elr1.isRotRight"
              @click="params.elr1.isRotRight = $event"
            />
          </template>
          <template v-else>
            &minus;
          </template>
        </div>
        <div class="elr2">
          <template v-if="devInfo.numElrs == 2">
            <toggle-button
              type="LR"
              :model="params.elr2.isRotRight"
              @click="params.elr2.isRotRight = $event"
            />
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          シュートパワー
          <div class="eng">Shoot power</div>
        </div>
        <div class="elr1">
          <template v-if="devInfo.isElrController()">
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
          </template>
          <template v-else>
            &minus;
          </template>
        </div>
        <div class="elr2">
          <template v-if="devInfo.numElrs == 2">
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
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          マニュアルモード
          <div class="eng">Manual mode</div>
        </div>
        <div class="elr1">
          <template v-if="devInfo.isElrController()">
            <toggle-button
              :model="params.elr1.enabledManual"
              @click="params.elr1.enabledManual = $event"
            />
          </template>
          <template v-else>
            &minus;
          </template>
        </div>
        <div class="elr2">
          <template v-if="devInfo.numElrs == 2">
            <toggle-button
              :model="params.elr2.enabledManual"
              @click="params.elr2.enabledManual = $event"
            />
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          オートモード
          <div class="eng">Auto mode</div>
        </div>
        <div class="common">
          <template v-if="devInfo.numElrs == 2">
            <toggle-button
              type="12"
              :model="params.elr2Auto"
              @click="params.elr2Auto = $event"
            />
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          猶予時間 [ms]
          <div class="eng">Latency</div>
        </div>
        <div class="common">
          <template v-if="devInfo.isElrController()">
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
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          遅延時間 [ms]
          <div class="eng">Delay</div>
        </div>
        <div class="common">
          <template v-if="devInfo.isElrController()">
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
          </template>
          <template v-else>
            &minus;
          </template>
        </div>

        <div class="title">
          BBP採用SP値を<br/>メインに表示
          <div class="eng">BBP SP is main</div>
        </div>
        <div class="common">
          <toggle-button
            :model="params.isBbpSpMain"
            @click="params.isBbpSpMain = $event"
          />
        </div>
      </div>


    </v-card-text>
  </v-card>

</template>


<style lang="scss" scoped>
.two-button-layout {
  display: grid;
  grid-template-columns: 1fr 1fr;
  column-gap: 0.6rem;
  margin-bottom: 1.5rem;
}

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
      color: #90A4AE;
    }
  }
  
  .note {
    margin-top: 0.2rem;
    font-size: 0.65rem;
    text-align: right;
    color: #90A4AE;
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
      background-color: #E3F2FD;
    }
  }
}

.buttons {
  display: grid;
  grid-template-columns: 1fr auto auto 1fr;
  column-gap: 0.5rem;
  
  .btn-read {
    grid-column: 2;
  }
  .btn-write {
    grid-column: 3;
  }
}
</style>

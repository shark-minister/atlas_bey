<script setup lang="ts">
import { useDisplay } from "vuetify"
import { useATLAS } from "@/classes/atlas"
import {
  mdiConnection,
  mdiSwapHorizontal,
  mdiRocketLaunch,
  mdiLanConnect,
  mdiLanDisconnect
} from "@mdi/js"

const {
  devInfo,
  // フラグ
  isConnected,
  // 操作
  requestDevice,
  connect,
  disconnect,
  readDeviceInfo,
  readParams,
  launchManually,
  switchToAutoMode
} = useATLAS()

async function startAtlas() {
  await requestDevice()
  await connect()
  await readDeviceInfo()
  await readParams()
}

const { mobile } = useDisplay()
</script>

<template>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="connection"
    :prepend-icon="mdiConnection"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        接続
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          ATLASとの接続を行います。ATLASがマニュアル・設定モードになっていないと接続できません。
        </div>
        <div>
          This establishes a connection with ATLAS.
          ATLAS must be in Manual/Configuration mode.
        </div>
      </div>

      <div class="connection-card-layout">
        <v-btn
          @click="startAtlas"
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiLanConnect"
          :disabled="isConnected"
        >
          <span>
            <div class="mb-1">
              接続する
            </div>
            <small class="btn-label-eng">
              connect
            </small>
          </span>
        </v-btn>

        <v-btn
          @click="disconnect"
          color="red"
          height="56"
          variant="flat"
          :prepend-icon="mdiLanDisconnect"
          :disabled="!isConnected"
        >
          <span>
            <div class="mb-1">
              切断する
            </div>
            <small class="btn-label-eng">
              disconnect
            </small>
          </span>
        </v-btn>
      </div>

    </v-card-text>
  </v-card>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="mode change"
    :prepend-icon="mdiSwapHorizontal"
    :width="mobile ? undefined : 500"
    :disabled="!devInfo.useSoftwareSwitch()"
  >
    <template v-slot:title>
      <div class="card-title">
        モード切替
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          オート・SP計測モードに切り替えます。切替に成功すると、このクライアントとの接続が自動で切れます。
        </div>
        <div>
          Switching to Auto/SP-measurement mode.
          If the switch is successful, the connection with this client will be terminated.
        </div>
      </div>

      <div class="mode-card-layout">
        <v-btn
          @click="switchToAutoMode"
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiSwapHorizontal"
          :disabled="!isConnected"
        >
          <span>
            <div class="mb-1">
              モードを切り替える
            </div>
            <small class="btn-label-eng">
              change mode
            </small>
          </span>
        </v-btn>
      </div>

    </v-card-text>
  </v-card>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="launch bey"
    :prepend-icon="mdiRocketLaunch"
    :width="mobile ? undefined : 500"
    :disabled="!devInfo.isElrController()"
  >
    <template v-slot:title>
      <div class="card-title">
        ベイ射出
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          モーターを駆動してベイを射出します。シュートパワーはパラメータ頁で設定が可能です。
        </div>
        <div>
          Drive motor(s) to launch your bey.
          The shoot power can be configured on the parameters page.
        </div>
      </div>

      <div class="mode-card-layout">
        <v-btn
          @click="launchManually"
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiRocketLaunch"
          :disabled="!isConnected"
        >
          <span>
            <div class="mb-1">
              ベイを射出する
            </div>
            <small class="btn-label-eng">
              launch bey manually
            </small>
          </span>
        </v-btn>
      </div>

    </v-card-text>
  </v-card>

</template>

<style lang="scss" scoped>
.connection-card-layout {
  display: grid;
  grid-template-columns: auto auto;
  column-gap: 0.6rem;
}

.mode-card-layout {
  display: grid;
  grid-template-columns: auto;
}
</style>
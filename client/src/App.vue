<script setup lang="ts">
import { useDisplay } from "vuetify"
import { mdiChartLine, mdiCellphoneWireless, mdiCog, mdiChartHistogram } from '@mdi/js'
import { useATLAS } from "./classes/atlas"

const {
  isConnected,
} = useATLAS()

const { mobile } = useDisplay()

</script>

<template>
  <v-app>

    <!-- PC用サイドバー -->
    <v-navigation-drawer
      v-if="!mobile"
      permanent
    >
      <v-list>
        <v-list-item
          to="/"
          title="接続"
        />
        
        <v-list-item
          to="/statistics"
          title="SP統計"
        />

        <v-list-item
          to="/profiles"
          title="SPプロファイル"
        />

        <v-list-item
          to="/parameters"
          title="パラメータ"
          :disabled="!isConnected"
        />
      </v-list>
    </v-navigation-drawer>

    <v-main class="bg-grey-lighten-3">
      <v-container>
        <router-view/>
      </v-container>
    </v-main>

    <!-- モバイル用ボトムタブ -->
    <v-bottom-navigation
      v-if="mobile"
      grow
    >
      <v-btn to="/">
        <v-icon :icon="mdiCellphoneWireless"/>
      </v-btn>

      <v-btn
        to="/statistics"
      >
        <v-icon :icon="mdiChartHistogram"/>
      </v-btn>

      <v-btn
        to="/profiles"
      >
        <v-icon :icon="mdiChartLine"/>
      </v-btn>

      <v-btn
        to="/parameters"
        :disabled="!isConnected"
      >
        <v-icon :icon="mdiCog"/>
      </v-btn>
    </v-bottom-navigation>

  </v-app>
</template>

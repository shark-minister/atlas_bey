<script setup lang="ts">
import { useATLAS } from '@/classes/atlas'
import { computed } from 'vue'
import { Bar } from "vue-chartjs"
import { useDisplay } from "vuetify"
import {
  mdiChartHistogram,
  mdiUpload,
  mdiDownload,
  mdiEraser
} from '@mdi/js'
import {
  Chart as ChartJS,
  BarElement,
  CategoryScale,
  LinearScale,
  Tooltip,
  type ChartData,
  type ChartOptions
} from 'chart.js'

// Chart.js の必要モジュールを登録
ChartJS.register(
  BarElement,
  CategoryScale,
  LinearScale,
  Tooltip
)

const {
  // データ
  result,
  isConnected,
  isGattBusy,
  readResult,
  clearResult,
} = useATLAS()

const { mobile } = useDisplay()

const stats = computed(() => {
  return result.value.statsEval
})

// グラフデータ
const chartData = computed<ChartData<'bar'>>(() => ({
  labels: stats.value.hist.label,
  datasets: [{
    data: stats.value.hist.data,
    backgroundColor: '#438ee4',
    barPercentage: 1.0
  }]
}))

// グラフオプション
const chartOptions = computed<ChartOptions<'bar'>>(() => ({
  indexAxis: 'y',
  responsive: true,
  maintainAspectRatio: false,
  scales: {
    x: {
      beginAtZero: true,
      title: {
        display: true,
        font: {
          size: 14,
          weight: "bold"
        },
        text: "シュート数 / num. shoots"
      },
      ticks: {
        font: {
          size: 13
        }
      }
    },
    y: {
      reverse: true,
      title: {
        display: true,
        font: {
          size: 14,
          weight: "bold"
        },
        text: "シュートパワー / shoot power"
      },
      ticks: {
        font: {
          size: 13
        }
      }
    }
  },
  plugins: {
    legend: {
      display: false
    }
  }
}))

async function confirmClearResult() {
  const msg = "デバイス内のデータを消去しますか？<br/>Are you share to clear data in the device."
  if (confirm(msg)) {
    await clearResult()
  }
}

const statsLoaded = computed(() => {
  return stats.value.total > 0
})

</script>

<template>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="shoot statistics"
    :prepend-icon="mdiChartHistogram"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        シュート統計データ
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          シュートパワー統計の読み出し・削除を行います。
        </div>
        <div>
          Read / clear the SP statistics.
        </div>
      </div>

      <div class="two-button-layout">

        <v-btn
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiUpload"
          :disabled="!isConnected || isGattBusy"
          @click="readResult()"
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
          :prepend-icon="mdiEraser"
          :disabled="!isConnected || isGattBusy"
          @click="confirmClearResult()"
        >
          <span>
            <div class="mb-1">
              クリア
            </div>
            <small class="btn-label-eng">
              clear
            </small>
          </span>
        </v-btn>

        <v-btn
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiDownload"
          :disabled="!statsLoaded"
          @click="stats.downloadFile('sp-distribution.csv')"
        >
          <span>
            <div class="mb-1">
              CSV出力
            </div>
            <small class="btn-label-eng">
              download
            </small>
          </span>
        </v-btn>

      </div>

      <!-- 統計データ -->
      <div class="stats-view">
        <div class="stats-item num-shoots">
          <div class='heading'>
            #
          </div>
          <div class='value'>
            {{ stats.total }}
          </div>
        </div>

        <div class="stats-item max-min-sp">
          <div class='heading'>
            Max / Min
          </div>
          <div class='value'>
            {{ stats.maxSP }}
            <span>/ {{ stats.minSP }}</span>
          </div>
        </div>

        <div class="stats-item mean-sp">
          <div class='heading'>
            Mean
          </div>
          <div class='value'>
            {{ stats.meanSP }}
            <span>&pm; {{ stats.stdevSP }}</span>
          </div>
        </div>
      </div>

      <!-- SPヒストグラム -->
      <div
        v-if="stats.total > 0"
        class="sp-chart-container"
      >
        <Bar
          :data="chartData"
          :options="chartOptions"
        />
      </div>

    </v-card-text>
  </v-card>

</template>


<style lang="scss" scoped>
.two-button-layout {
  display: grid;
  grid-template-columns: auto auto auto;
  column-gap: 0.2rem;
}

.stats-view {
  display: grid;
  grid-template-rows: auto;
  grid-template-columns: auto 1fr 1fr;
  margin-top: 1.5rem;
  margin-bottom: 0.5rem;

  .stats-item {
    background-color: #EEEEEE;
    padding: 0.5rem;
    display: inline-block;
    margin-bottom: 0.5rem;
    margin-right: 1px;

    &:last-child {
      margin-right: 0;
    }
    &.num-shoots {
      grid-column: 1;
    }
    &.max-min-sp {
      grid-column: 2;
    }
    &.mean-sp {
      grid-column: 3;
    }

    .heading {
      font-size: 0.7rem;
      font-style: italic;
      opacity: 0.5;
      margin-bottom: 0.1rem;
    }
    .value {
      display: inline-block;
      font-size: 1.4rem;
      font-weight: bold;

      span {
        font-size: 0.8rem;
      }
    }
  }
}

.sp-chart-container {
  height: 22rem;
  width: 100%;
}
</style>
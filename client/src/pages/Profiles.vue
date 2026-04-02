<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import {
  Chart as ChartJS,
  LinearScale,
  PointElement,
  LineElement,
  Tooltip,
  Legend,
  Title,
  type ChartData,
  type ChartOptions
} from "chart.js"
import { Scatter } from 'vue-chartjs'
import { SPProfile, type Point } from '@/classes/sp-profile'
import { useDisplay } from "vuetify"
import {
  mdiChartLine,
  mdiViewList,
  mdiUpload,
  mdiDownload
} from '@mdi/js'
import { useATLAS } from '@/classes/atlas'
//import { testData } from '@/classes/test'

// Chart.js の必要モジュールを登録
ChartJS.register(
  LinearScale,
  PointElement,
  LineElement,
  Tooltip,
  Legend,
  Title
)

const {
  profManager,
  isConnected,
  isGattBusy,
  readRawData,
} = useATLAS()

const selectedIndex = ref(0xFFFFFFFF)

const headers =  [
  {
    title: '#',
    align: 'end',
    key: 'entry'
  },
  {
    title: 'SP',
    align: 'end',
    key: 'evalSP'
  },
  {
    title: 'BBP',
    align: 'end',
    key: 'origSP'
  },
  {
    title: 'Acc',
    align: 'end',
    key: 'acc1'
  },
  {
    title: "Index",
    align: 'end',
    key: "index",
    headerProps: {
      class: 'hidden-col'
    },
    cellProps: {
      class: 'hidden-col'
    }
  }
] as const

const shootList = computed(() => {
  const profs = profManager.value.profiles
  if (!profs) {
    return []
  }
  return profs!.map((prof, index) => {
    return {
      entry: prof.entryNumber,
      evalSP: prof.myEvalSP,
      origSP: prof.origSP,
      acc1: Math.round(prof.regParam1.a),
      acc2: Math.round(prof.regParam2.a),
      index: index
    }
  })
})

const listLoaded = computed(() => {
  return profManager.value.numShoots() > 0
})

//profManager.value.set(testData)

const { mobile } = useDisplay()

//-----------------------------------------------------------------------------

const chartProps = computed(() => {
  if (mobile.value) {
    return {
      lineWidth: 2,
      regLineWidth: 1,
      pointRadius: 3,
      axisLabelSize: 15,
      ticksLabelSize: 13,
      ticksLength: 2,
      titleSize: 18
    }
  }
  else {
    return {
      lineWidth: 3,
      regLineWidth: 2,
      pointRadius: 7,
      axisLabelSize: 18,
      ticksLabelSize: 15,
      ticksLength: 5,
      titleSize: 22
    }
  }
})

const prof = computed<SPProfile | null>(() => {
  return profManager.value.getProfile(selectedIndex.value)
})

const sortBy = ref()
onMounted(() => {
  sortBy.value = [{ key: 'entry', order: 'asc' }]
})

//-----------------------------------------------------------------------------
// プロファイル
//-----------------------------------------------------------------------------

// グラフデータ
const prof1ChartData = computed<ChartData<"scatter">>(() => {
  if (!prof.value) {
    return {
      datasets: []
    }
  }

  // フィッティング
  const p = prof.value.points1[prof.value.points1.length - 1]!

  return {
    datasets: [
      {
        data: prof.value.points1,
        // ラインとマーカー
        pointStyle: 'cross',
        pointRadius: chartProps.value.pointRadius,
        showLine: false,
        borderWidth: chartProps.value.lineWidth,
        borderColor: "#1E88E5",
      },
      {
        data: [
          { x: 0, y: prof.value.regParam1.b },
          { x: p.x, y: prof.value.regParam1.a * p.x + prof.value.regParam1.b }
        ],
        // ラインとマーカー
        pointRadius: 0,
        showLine: true,
        borderWidth: chartProps.value.regLineWidth,
        borderColor: "#FFA726",
      },
    ]
  }
})

const prof2ChartData = computed<ChartData<"scatter">>(() => {
  if (!prof.value) {
    console.log("undefined")
    return {
      datasets: []
    }
  }

  return {
    datasets: [
      {
        data: prof.value.points1,
        // ラインとマーカー
        pointStyle: 'cross',
        pointRadius: chartProps.value.pointRadius,
        showLine: true,
        borderWidth: chartProps.value.lineWidth,
        borderColor: "#1E88E5",
      },
      {
        data: prof.value.points2,
        // ラインとマーカー
        pointStyle: 'cross',
        pointRadius: chartProps.value.pointRadius,
        showLine: true,
        borderWidth: chartProps.value.lineWidth,
        borderColor: "#BBDEFB"
      }
    ]
  }
})

const profChartOptions = computed<ChartOptions<'scatter'>>(() => {
  if (!prof.value) {
    return {}
  }

  return {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        type: "linear",
        beginAtZero: true,
        title: {
          display: true,
          font: {
            size: chartProps.value.axisLabelSize,
            //weight: 'bold'
          },
          text: "経過時間 [ms]"
        },
        ticks: {
          font: {
            size: chartProps.value.ticksLabelSize
          }
        },
        grid: {
          drawOnChartArea: false,
          tickLength: chartProps.value.ticksLength
        }
      },
      y: {
        min: 0,
        title: {
          display: true,
          font: {
            size: chartProps.value.axisLabelSize,
            //weight: 'bold'
          },
          text: "シュートパワー [rpm]"
        },
        ticks: {
          font: {
            size: chartProps.value.ticksLabelSize
          }
        },
        grid: {
          tickLength: chartProps.value.ticksLength
        }
      }
    },
    plugins: {
      legend: {
        display: false
      },
      title: {
        display: false,
      }
    }
  }
})

//-----------------------------------------------------------------------------
// 加速度 vs SP
//-----------------------------------------------------------------------------

// グラフデータ
const accVsSPChartData = computed<ChartData<"scatter">>(() => {
  const L = profManager.value.profiles.length
  if (L === 0) {
    return {
      datasets: []
    }
  }

  const d1 = new Array<Point>(L)
  const d2 = new Array<Point>(L)
  for (let i = 0; i < L; ++i) {
    const p = profManager.value.profiles[i]!
    d1[i] = {
      x: p.regParam1.a,
      y: p.myEvalSP
    }
    d2[i] = {
      x: p.regParam2.a,
      y: p.myEvalSP
    }
  }

  return {
    datasets: [
      {
        data: d1,
        // ラインとマーカー
        pointStyle: 'cross',
        pointRadius: chartProps.value.pointRadius,
        showLine: false,
        borderWidth: chartProps.value.lineWidth,
        borderColor: "#1E88E5",
      },
      {
        data: d2,
        // ラインとマーカー
        pointStyle: 'cross',
        pointRadius: chartProps.value.pointRadius,
        showLine: false,
        borderWidth: chartProps.value.lineWidth,
        borderColor: "#FFA726",
      }
    ]
  }
})

const accVsSPChartOptions = computed<ChartOptions<'scatter'>>(() => {
  if (profManager.value.profiles.length === 0) {
    return {}
  }

  return {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        //type: "linear",
        beginAtZero: false,
        title: {
          display: true,
          font: {
            size: chartProps.value.axisLabelSize,
          },
          text: "シュート加速度 [rpm/ms]"
        },
        ticks: {
          font: {
            size: chartProps.value.ticksLabelSize
          }
        },
        grid: {
          drawOnChartArea: false,
          tickLength: chartProps.value.ticksLength
        }
      },
      y: {
        beginAtZero: false,
        title: {
          display: true,
          font: {
            size: chartProps.value.axisLabelSize,
          },
          text: "最終シュートパワー [rpm]"
        },
        ticks: {
          font: {
            size: chartProps.value.ticksLabelSize
          }
        },
        grid: {
          tickLength: chartProps.value.ticksLength
        }
      }
    },
    plugins: {
      legend: {
        display: false
      },
      title: {
        display: false
      }
    }
  }
})

</script>

<template>
  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="detailed shoot list"
    :prepend-icon="mdiViewList"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        シュート詳細一覧
      </div>
    </template>

    <v-card-text>
      <div class="card-description">
        <div>
          シュート詳細データ一覧の読み出しを行います。
        </div>
        <div>
          Read detailed shoot list.
        </div>
      </div>

      <div class="two-button-layout">
        <v-btn
          height="56"
          color="primary"
          variant="flat"
          :prepend-icon="mdiUpload"
          :disabled="!isConnected || isGattBusy"
          @click="readRawData()"
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
          :disabled="!listLoaded"
          @click="profManager.downloadFile('profile.dat')"
        >
          <span>
            <div class="mb-1">
              ダウンロード
            </div>
            <small class="btn-label-eng">
              download
            </small>
          </span>
        </v-btn>
      </div>

      <!-- シュート一覧 -->
      <div class="shoot-list">
        <v-data-table
          density="compact"
          class="shoot-list-table"
          :headers="headers"
          :items="shootList"
          :items-per-page="5"
          item-value="index"
          :fixed-footer="true"
          :sort-by="sortBy"
        >
          <!-- 行描画 -->
          <template #item="{ item, props }">
            <tr
              v-bind="props"
              :class="{
                'selected-row': item.index === selectedIndex
              }"
              @click="selectedIndex = item.index"
            >
              <td>{{ item.entry }}</td>
              <td>{{ item.evalSP }}</td>
              <td>{{ item.origSP }}</td>
              <td>{{ item.acc1 }}</td>
            </tr>
          </template>

        </v-data-table>
      </div>

    </v-card-text>
  </v-card>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="shoot power profile"
    :prepend-icon="mdiChartLine"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        SPプロファイル
      </div>
    </template>

    <v-card-text>

      <div
        v-if="prof"
        class="chart"
      >
        <div class="chart-title">
          <div class="entry">
            <span># </span>{{ prof.entryNumber }}
          </div>

          <div class="sp">
            <span>SP: </span>{{ prof.myEvalSP }}
          </div>
          <div class="accel">
            <span>加速度: </span>{{ Math.round(prof.regParam1.a) }} / {{ Math.round(prof.regParam2.a) }}
          </div>
        </div>

        <!-- プロファイルチャート -->
        <div class="prof1-chart">
          <Scatter
            :data="prof1ChartData"
            :options="profChartOptions"
          />
        </div>

        <!-- プロファイルチャート -->
        <div class="prof2-chart">
          <Scatter
            :data="prof2ChartData"
            :options="profChartOptions"
          />
        </div>
      </div>

    </v-card-text>
  </v-card>

  <v-card
    variant="flat"
    class="mx-auto card-setting"
    subtitle="shoot power acceleration"
    :prepend-icon="mdiChartLine"
    :width="mobile ? undefined : 500"
  >
    <template v-slot:title>
      <div class="card-title">
        SP加速度
      </div>
    </template>

    <v-card-text>
      <div
        v-if="listLoaded"
        class="chart"
      >
        <div class="chart-title">
          <div class="sp">
            中盤 <span>middle</span>
          </div>
          <div class="accel">
            最後4回転 <span>last 4 rots</span>
          </div>
        </div>

        <div class="accel-chart">
          <Scatter
            :data="accVsSPChartData"
            :options="accVsSPChartOptions"
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
  margin-bottom: 1rem;
}

.selected-row {
  background-color: #E3F2FD !important;
}

.buttons {
  display: grid;
  grid-template-columns: 1fr 1fr;
  column-gap: 0.5rem;
}

.chart {
  .chart-buttons {
    display: grid;
    grid-template-columns: 1fr 1fr;
    column-gap: 0.5rem;
    margin-bottom: 0.5rem;
  }

  .chart-title {
    margin-top: 1rem;
    margin-bottom: 0.5rem;
    display: grid;
    grid-template-columns: 1fr auto auto auto 1fr;
    column-gap: 1rem;

    div {
      font-size: 1.1rem;
      font-weight: bold;

      &.entry {
        grid-column: 2;
      }

      &.sp {
        grid-column: 3;
        color: #1E88E5;
      }
      &.accel {
        grid-column: 4;
        color: #FB8C00;
      }

      span {
        font-weight: normal;
        font-size: 0.8rem;
      }
    }
  }

  .prof1-chart,
  .prof2-chart {
    width: 100%;
    height: 18rem;
  }

  .accel-chart {
    width: 100%;
    height: 18rem;
  }

  .chart-invisible {
    display: none;
  }
}

.shoot-list-table {
  font-size: 0.8rem;
  ::v-deep(.v-data-table-footer__items-per-page) {
    display: none;
  }
}


// PCサイト
.content-pc {

  .chart {
    display: grid;
    grid-template-rows: auto 1fr;
    grid-template-columns: 1fr 1fr;
    column-gap: 1rem;

    .chart-title {
      grid-row: 1;
      grid-column: 1 / span 2;
      margin-top: 1rem;
      margin-bottom: 0.5rem;
      display: grid;
      grid-template-columns: 1fr auto auto auto 1fr;
      column-gap: 1rem;

      div {
        font-size: 1.1rem;
        font-weight: bold;

        &.entry {
          grid-column: 2;
        }

        &.sp {
          grid-column: 3;
          color: rgb(50, 100, 255);
        }
        &.accel {
          grid-column: 4;
          color: orange;
        }

        span {
          font-weight: normal;
          font-size: 0.8rem;
        }
      }
    }

    .prof1-chart {
      grid-row: 2;
      grid-column: 1;
      width: 100%;
      height: 30rem;
    }

    .prof2-chart {
      grid-row: 2;
      grid-column: 2;
      width: 100%;
      height: 30rem;
    }
  }

  .accel-chart {
    width: 100%;
    height: 30rem;
  }

  .shoot-list-table {
    margin: auto;
    width: 30rem;
    font-size: 1rem;
    ::v-deep(.v-data-table-footer__items-per-page) {
      display: none;
    }
  }
  
}

::v-deep(.v-table__wrapper)
  > table
  > thead
  > tr
  > th:not(.v-data-table__th--sorted):not(:hover)
  .v-data-table-header__sort-icon {
  opacity: 0.2; /* $data-table-header-sort-icon-default-opacity */
}
::v-deep(.v-data-table-header__sort-icon) {
  margin-inline: 0.5rem 0; /* $data-table-header-sort-icon-margin-inline */
}
</style>

<style lang="scss">

.hidden-col {
  display: none;
}
</style>
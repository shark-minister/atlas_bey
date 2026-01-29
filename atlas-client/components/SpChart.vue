<!--
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
-->
<script setup lang="ts">
import { computed } from 'vue'
import { Histogram } from '../classes/histogram'
import { Bar } from "vue-chartjs"
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

// プロパティの定義
const props = defineProps<{
  hist: Histogram
}>()

console.log("test")

// グラフデータ
const chartData = computed<ChartData<'bar'>>(() => ({
  labels: props.hist.label,
  datasets: [{
    data: props.hist.data,
    backgroundColor: '#008fd1'
  }]
}))

// グラフオプション
const chartOptions = computed<ChartOptions<'bar'>>(() => ({
  indexAxis: 'y', // 👈 横棒グラフにするポイント
  responsive: true,
  maintainAspectRatio: false,
  scales: {
    x: {
      beginAtZero: true,
      title: {
        display: true,
        font: {
          size: 14
        },
        text: "シュート数 / num. shoots"
      }
    },
    y: {
      reverse: true,
      title: {
        display: true,
        font: {
          size: 14
        },
        text: "シュートパワー / shoot power"
      }
    }
  }
}))

</script>

<template>
  <Bar
    :data="chartData"
    :options="chartOptions"
  />
</template>

<style lang='scss' scoped>
</style>

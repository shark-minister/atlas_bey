<!--
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
-->
<script setup lang="ts">
import { ref } from 'vue'

// プロパティの定義
const {
  model,
  type = 'OnOff'
} = defineProps<{
  model: boolean
  type?: string
}>()

// イベントの定義
const emit = defineEmits(['click'])

const value = ref(model)
const changeFlag = () => {
  emit('click', value)
}

</script>

<template>
  <label
    class="toggle-btn"
    :class="`type-${type}`"
  >
    <input
      type="checkbox"
      v-model="value"
      @change="changeFlag"
    />
  </label>
</template>

<style lang='scss' scoped>
$toggleSize: 1.5rem;

// 共通設定
.toggle-btn {
  // 配置
  display: inline-block;
  position: relative;
  margin: auto;
  width: calc($toggleSize * 2);
  height: $toggleSize;
  // テキスト
  font-weight: bold;
  text-align: center;
  line-height: $toggleSize;
  // 装飾
  border-radius: $toggleSize;
  cursor: pointer;
  transition: background-color .5s;

  &:has(:checked)::after {
    left: $toggleSize;
  }

  &::after {
    position: absolute;
    top: 0;
    left: 0;
    width: $toggleSize;
    height: $toggleSize;
    border-radius: 50%;
    box-shadow: 0 0 5px rgb(0 0 0 / 20%);
    background-color: #fff;
    transition: left .5s;
  }

  input {
    display: none;
  }
}

// ON/OFF
.type-OnOff {
  background-color: #dfdfdf;
  &:has(:checked) {
    background-color: #008fd1;
  }
  &::after {
    content: '';
  }
}

// L/R
.type-LR {
  color: #cf6b00;
  background-color: #fb8200;
  &:has(:checked) {
    background-color: #008fd1;
    &::after {
      content: 'R';
      color: #0086c0;
    }
  }
  &::after {
    content: 'L';
  }
}

// 1/2
.type-12 {
  color: #008fd1;
  background-color: #008fd1;
  &:has(:checked) {
    background-color: #fb8200;
    &::after {
      content: '2';
      color: #cf6b00;
    }
  }
  &::after {
    content: '1';
  }
}
</style>

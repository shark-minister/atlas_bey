<!--
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
-->
<script setup lang="ts">

// プロパティの定義
defineProps<{
  labelJpn: string
  labelEng: string
  disabled: boolean
}>()

// イベントの定義
const emit = defineEmits(['click']);
</script>

<template>
  <button
    @click="emit('click')"
    :disabled="disabled"
  >
    {{ labelJpn }}
    <div class="eng">{{ labelEng }}</div>
  </button>
</template>

<style lang='scss' scoped>
// 戻るアイコン（＜）
button {
  padding: 0.5rem;
  font-size: 0.9rem;
  background-color: rgb(193, 193, 193);

  // 装飾
  border: none;
  border-radius: 0.3rem;
  box-shadow: none;

  .eng {
    margin-top: 0.2rem;
    font-size: 0.8rem;
    color: white;
  }

  &:disabled {
    opacity: 60%;
  }
}
</style>

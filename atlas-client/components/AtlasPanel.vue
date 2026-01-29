<script setup lang="ts">

// プロパティの定義
defineProps<{
  titleJpn: string
  titleEng: string
}>()

</script>

<template>
  <div class="wrapper">
    <h2>
      {{ titleJpn }}
      <span>{{ titleEng }}</span>
    </h2>
    <div class="container">
      <slot name="content"/>
    </div>
  </div>
</template>

<style lang="scss" scoped>
.wrapper {
  margin: 0.24rem;
  background-color: white;
  box-shadow: 0 0 0.2rem 0.2rem rgb(0 0 0 / 10%);
  height: fit-content;

  h2 {
    margin: 0;
    background-color: #0077ae;
    color: white;
    font-size: 1.1rem;
    padding: 1rem;

    span {
      font-size: smaller;
      margin-left: 0.4rem;
      opacity: 50%;
    }
  }

  .container {
    padding: 1rem;
  }
}
</style>

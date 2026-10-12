---
layout: cover
background: /img/header-bg.svg
---

# Cache Ping-Pong


---
colorSchema: light
---

# Cache Ping-Pong

<img src="/img/cache/cache-ping-pong-1.png" class="h-80 mx-auto"/>

---
colorSchema: light
---

# Cache Ping-Pong

<img src="/img/cache/cache-ping-pong-2.png" class="h-80 mx-auto"/>

---
colorSchema: light
---

# Cache Ping-Pong

<img src="/img/cache/cache-ping-pong-3.png" class="h-80 mx-auto"/>

---
colorSchema: light
---

# Cache Ping-Pong

<img src="/img/cache/cache-ping-pong-4.png" class="h-80 mx-auto"/>

---
colorSchema: light
---

# Cache Ping-Pong

<img src="/img/cache/cache-ping-pong-5.png" class="h-80 mx-auto"/>

---
layout: center
colorSchema: light
---

# Cache Ping-Pong - Solution

---
colorSchema: light
---

# Solution

<img src="/img/cache/cache-ping-pong-solution.png" class="h-80 mx-auto"/>

---
colorSchema: light
---

# Code

```cpp
struct Data
{
    alignas(std::hardware_destructive_interference_size) std::atomic<int> v1;
    alignas(std::hardware_destructive_interference_size) std::atomic<int> v2;
};
```

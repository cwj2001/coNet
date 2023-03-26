
# 压测报告

## 机器配置
服务器机器：物理内存4G,2核，硬盘20G
压测机器和运行服务器的机器是同一台

## 第一次压测，无优化
16.41K/s
![img_4.png](img_4.png)
16.71K/s
![img_7.png](img_7.png)
17.09K/s
![img_8.png](img_8.png)

## 加入内存池
16.92K/s
![img_3.png](img_3.png)
16.58K/s
![img_5.png](img_5.png)
17.11K/s
![img_6.png](img_6.png)

## 加入协程池
21.75K/s
![img_11.png](img_11.png)
19.90K/s
![img_9.png](img_9.png)
20.51K/s
![img_10.png](img_10.png)

### 去掉中心协程池

21.45K/s
![img_12.png](img_12.png)
21.66K/s
![img_13.png](img_13.png)

## 加入多级并列消息队列

锁竞争减少延时降低了，但是每秒处理数减少了（后面通过加入意向机制，解决了该现象）

### 2W连接数

![img_25.png](img_25.png)
![img_26.png](img_26.png)

### 3W连接数

![img_27.png](img_27.png)
![img_28.png](img_28.png)

### 加入意向机制后

延时降低，qps变大

#### 2W连接数
![img_29.png](img_29.png)
![img_33.png](img_33.png)
#### 3W连接数

![img_31.png](img_31.png)
![img_30.png](img_30.png)

## 连接数级别测试

### 1K数

21.74K/s
![img_11.png](img_11.png)

### 5K数

21.51K/s
![img_20.png](img_20.png)

### 1W数

![img_14.png](img_14.png)

### 2W数

21.01k/s
![img_32.png](img_32.png)
20.53K/s
![img_17.png](img_17.png)
15.09K/s
![img_15.png](img_15.png)
20.19k/s
![img_18.png](img_18.png)

-----

**注：以下级别压测10s是没法得出结果的，因为建立连接的时间可能不止10s,且结果不是特别稳定，但是恢复2W级别以及以下，服务器均会恢复正常**

### 3W数

#### 10S无结果
![img_16.png](img_16.png)

#### 3w 18.09K/s

![img_22.png](img_22.png)

3w 18.90k/s

![img_24.png](img_24.png)

### 4W 19.11K/s

![img_23.png](img_23.png)

### 6W数

![img_21.png](img_21.png)



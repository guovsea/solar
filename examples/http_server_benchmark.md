# 控制变量
访问相同的不存在的地址，让被测服务器回内容长度相同的 html 文本：

```html
<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>solar/1.0.0</center></body></html>
```

# 压测工具

# 未开启长连接情况下的压测数据
## http_server 压测数据 (1 线程)

```
Server Software:        solar/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8020

Document Path:          /solar
Document Length:        138 bytes

Concurrency Level:      200
Time taken for tests:   10.340 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Total transferred:      251000000 bytes
HTML transferred:       138000000 bytes
Requests per second:    96709.99 [#/sec] (mean)
Time per request:       2.068 [ms] (mean)
Time per request:       0.010 [ms] (mean, across all concurrent requests)
Transfer rate:          23705.28 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.3      1      13
Processing:     0    1   0.4      1      13
Waiting:        0    1   0.3      1       9
Total:          1    2   0.5      2      15

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      2
  80%      2
  90%      2
  95%      3
  98%      4
  99%      4
 100%     15 (longest request)
```

## http_server 压测数据 (2 线程)

```
Server Software:        solar/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8020

Document Path:          /solar
Document Length:        138 bytes

Concurrency Level:      200
Time taken for tests:   11.537 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Total transferred:      251000000 bytes
HTML transferred:       138000000 bytes
Requests per second:    86677.77 [#/sec] (mean)
Time per request:       2.307 [ms] (mean)
Time per request:       0.012 [ms] (mean, across all concurrent requests)
Transfer rate:          21246.21 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.4      1      18
Processing:     0    2   0.8      1      22
Waiting:        0    1   0.7      1      19
Total:          0    2   1.0      2      30
WARNING: The median and mean for the processing time are not within a normal deviation
        These results are probably not that reliable.

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      3
  80%      3
  90%      4
  95%      4
  98%      4
  99%      4
 100%     30 (longest request)
```

## http_server 压测数据 (3 线程)

```
Server Software:        solar/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8020

Document Path:          /solar
Document Length:        138 bytes

Concurrency Level:      200
Time taken for tests:   11.732 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Total transferred:      251000000 bytes
HTML transferred:       138000000 bytes
Requests per second:    85238.73 [#/sec] (mean)
Time per request:       2.346 [ms] (mean)
Time per request:       0.012 [ms] (mean, across all concurrent requests)
Transfer rate:          20893.48 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.4      1       8
Processing:     0    2   0.8      1      12
Waiting:        0    1   0.7      1      10
Total:          0    2   0.9      2      12
WARNING: The median and mean for the processing time are not within a normal deviation
        These results are probably not that reliable.

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      3
  80%      3
  90%      4
  95%      4
  98%      5
  99%      5
 100%     12 (longest request)
```


## nginx (1 个线程)

```
Server Software:        nginx/1.18.0
Server Hostname:        127.0.0.1
Server Port:            80

Document Path:          /solar
Document Length:        162 bytes

Concurrency Level:      200
Time taken for tests:   15.495 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Total transferred:      321000000 bytes
HTML transferred:       162000000 bytes
Requests per second:    64538.38 [#/sec] (mean)
Time per request:       3.099 [ms] (mean)
Time per request:       0.015 [ms] (mean, across all concurrent requests)
Transfer rate:          20231.27 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.3      1      10
Processing:     0    2   0.4      2      10
Waiting:        0    1   0.3      1       9
Total:          1    3   0.4      3      12

Percentage of the requests served within a certain time (ms)
  50%      3
  66%      3
  75%      3
  80%      3
  90%      3
  95%      3
  98%      4
  99%      5
 100%     12 (longest request)
```

# 开启长连接情况下的压测数据

## http_server (1 个线程)

```
Server Software:        solar/1.0.0
Server Hostname:        127.0.0.1
Server Port:            8020

Document Path:          /solar
Document Length:        138 bytes

Concurrency Level:      200
Time taken for tests:   6.514 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Keep-Alive requests:    1000000
Total transferred:      256000000 bytes
HTML transferred:       138000000 bytes
Requests per second:    153517.37 [#/sec] (mean)
Time per request:       1.303 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          38379.34 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       6
Processing:     0    1   0.2      1       7
Waiting:        0    1   0.2      1       7
Total:          0    1   0.2      1       7

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      1
  98%      2
  99%      2
 100%      7 (longest request)
```

## nginx (1 个线程)

```
Server Software:        nginx/1.18.0
Server Hostname:        127.0.0.1
Server Port:            80

Document Path:          /solar
Document Length:        162 bytes

Concurrency Level:      200
Time taken for tests:   2.410 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Keep-Alive requests:    990095
Total transferred:      325950475 bytes
HTML transferred:       162000000 bytes
Requests per second:    414915.89 [#/sec] (mean)
Time per request:       0.482 [ms] (mean)
Time per request:       0.002 [ms] (mean, across all concurrent requests)
Transfer rate:          132072.30 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0      11
Processing:     0    0   0.2      0      12
Waiting:        0    0   0.2      0       8
Total:          0    0   0.2      0      12

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      1
  95%      1
  98%      1
  99%      1
 100%     12 (longest request)
```
## libevent

```
Server Software:        
Server Hostname:        127.0.0.1
Server Port:            8030

Document Path:          /solar
Document Length:        138 bytes

Concurrency Level:      200
Time taken for tests:   10.991 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Keep-Alive requests:    0
Total transferred:      266000000 bytes
HTML transferred:       138000000 bytes
Requests per second:    90979.87 [#/sec] (mean)
Time per request:       2.198 [ms] (mean)
Time per request:       0.011 [ms] (mean, across all concurrent requests)
Transfer rate:          23633.44 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.3      1      16
Processing:     0    1   0.5      1      18
Waiting:        0    1   0.4      1      17
Total:          1    2   0.6      2      24

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      2
  80%      2
  90%      2
  95%      3
  98%      4
  99%      4
 100%     24 (longest request)
```
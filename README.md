# MocliaWebhook

接收Github Push Webhook 解析后传递给QQ群。

# 初期配置

## MiraiAPIHttp
```yaml
adapters: // 必须 
  - http
  - ws
debug: false
enableVerify: false // 可选true/false
verifyKey: INITKEYli8RR385 //上面选true这边要填
singleMode: true // 可选true/false
cacheSize: 4096
adapterSettings: // host和port必须相同
  http:
    host: localhost
    port: 5036
  
  ws:
    host: localhost
    port: 5036
    reservedSyncId: -1
```

## MocliaWebhook
在程序相同目录下创建config.json（json不支持注释，复制时请删除注释）
```json
 {
    "hostname": "localhost", // 与MiraiApiHttp adapterSettings的host和port设置相同
    "port": 5036,
    "botQQ": 2556410868, // 想连接的机器人账号
    "verifyKey": "Hello", // 与上面verifyKey相同，enableVerify为true时必填
    "enableVerify": false, // 同MiraiApiHttp设置
    "singleMode": true, // 同MiraiApiHttp设置
    "reservedSyncId": "-1", // 同MiraiApiHttp设置
    "threadPoolSize": 4, // MiraiCpp的线程数
    "sendGroup": 913854660 // 要将commit发送到的群聊
}

## GitHub
在仓库webhook设置中设置发送applicatio/json，连接地址填写http://服务器IP:端口/webhook

```
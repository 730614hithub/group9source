import mqtt from'../../utils/mqtt.js';
const aliyunOpt = require('../../utils/aliyun/aliyun_connect.js');

let that = null;

Page({
    data:{

     
      temperature:0,
      humidity:0,
      LightSwitch:0,

      client:null,
      reconnectCounts:0,
      options:{
        protocolVersion: 4, 
        clean: false,
        reconnectPeriod: 1000, 
        connectTimeout: 30 * 1000, 
        resubscribe: true,
        clientId: '',
        password: '',
        username: '',
      },

      aliyunInfo: {
                productKey: 'k17fimjhRbe', 
                deviceName: 'wechat', 
                deviceSecret: '194ac11a1c8bcdd798a0ff52bea1baa0', 
                regionId: 'cn-shanghai', 
                pubTopic: '/k17fimjhRbe/wechat/user/wechat',
                subTopic: '/k17fimjhRbe/wechat/user/get', 
              },
            },
    
    Things1CMD(a){
      var status=a.detail.value
      var that=this
      
     
         if(status==true){
          that.sendCommond(1);
         
         }else{
          that.sendCommond(0);
           
          
         }
    },

  onLoad:function(){
    that = this;
    let clientOpt = aliyunOpt.getAliyunIotMqttClient({
      productKey: that.data.aliyunInfo.productKey,
      deviceName: that.data.aliyunInfo.deviceName,
      deviceSecret: that.data.aliyunInfo.deviceSecret,
      regionId: that.data.aliyunInfo.regionId,
      port: that.data.aliyunInfo.port,
    });

    console.log("get data:" + JSON.stringify(clientOpt));
    let host = 'wxs://' + clientOpt.host;
    
    this.setData({
      'options.clientId': clientOpt.clientId,
      'options.password': clientOpt.password,
      'options.username': clientOpt.username,
    })
    console.log("this.data.options host:" + host);
    console.log("this.data.options data:" + JSON.stringify(this.data.options));

    //访问服务器
    this.data.client = mqtt.connect(host, this.data.options);

    this.data.client.on('connect', function (connack) {
      wx.showToast({
        title: '连接成功'
      })
      console.log("连接成功");
    })

    
    that.data.client.on("message", function (topic, payload) {
      
      let dataFromALY = {};
      try {
        dataFromALY = JSON.parse(payload);
        console.log(dataFromALY);
       that.setData({
        
        LightSwitch:dataFromALY.params.LightSwitch,
        humidity:dataFromALY.params.humi,
        temperature:dataFromALY.params.temp,
      })
      } catch (error) {
        console.log(error);
      }
    })

    
    that.data.client.on("error", function (error) {
      console.log(" 服务器 error 的回调" + error)

    })
    
    that.data.client.on("reconnect", function () {
      console.log(" 服务器 reconnect的回调")

    })
    
    that.data.client.on("offline", function (errr) {
      console.log(" 服务器offline的回调")
    })
  },
  
  onClickOpen() {
    that.sendCommond(1);
  },
  onClickOff() {
    that.sendCommond(0);
  },
  sendCommond(data) {
    let sendData = {
      LightSwitch: data,
    };
    

  
    
    
    if (this.data.client && this.data.client.connected) {
      this.data.client.publish(this.data.aliyunInfo.pubTopic, JSON.stringify(sendData));
      console.log(this.data.aliyunInfo.pubTopic)
      console.log(JSON.stringify(sendData))
    } else {
      wx.showToast({
        title: '请先连接服务器',
        icon: 'none',
        duration: 2000
      })
    }
  }
})

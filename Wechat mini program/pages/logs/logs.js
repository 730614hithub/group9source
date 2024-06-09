Page({  
  data: {
        weatherData: {
          airQuality: "", 
          location:"",
          temperature: "",
          weather: ""
        }
      },
      onLoad() {
        
        this.getWeather();
      },
      getWeather() {
        const that = this; 
        wx.request({
          url: 'https://devapi.qweather.com/v7/weather/now',
          data: {
            key: "b2d75af44d054e929241a99a990e4fec",
            location: "119.57,39.95"
          },
          success: function (res) {
            console.log(res);
            that.setData({
              
              'weatherData.temperature': res.data.now.temp,
              'weatherData.weather': res.data.now.text,
              'weatherData.airQuality': "良好", 
              'weatherData.location': "河北省-秦皇岛市"
            });
          },
          fail: function (res) {
            console.log("请求失败", res);
          }
        });
      },
      changeImage: function (e) {
        wx.reLaunch({
          url: '/pages/logs/logs'
        })
      },
  info: function(e) {  
    wx.redirectTo({  
      url: '/pages/detail/detail'  
    });  
  }  
});

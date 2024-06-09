Page({  
  
  data: {  
    gender: 'male',  
    username: 'Cesar',  
    imgUrl: "/static/微信图片_20240421112545.jpg",  
    
    selectedImgUrl: ''  
  },  
  
  
  
  jump: function(e) {  
    wx.navigateTo({  
      url: '/pages/modife/modife?username=' + encodeURIComponent(this.data.username) +  
        '&gender=' + encodeURIComponent(this.data.gender)  
    });  
  }  
  
});

  

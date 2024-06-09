Page({  
  data: {  
    username: '',  
    gender: ' '  
  },  
  onLoad: function(options) {  
    
    if (options.username) {  
      this.setData({  
        username: decodeURIComponent(options.username)  
      });  
    }  
    if (options.gender) {  
      this.setData({  
        gender: decodeURIComponent(options.gender)  
      });  
    }  
  },  
  formSubmit: function(e) {  
    var formData = e.detail.value;  
    var pages = getCurrentPages();  
    // 确保页面栈中至少有两个页面  
    if (pages.length > 1) {  
      var prePage = pages[pages.length - 2];  
      // 更新前一个页面的数据  
      prePage.setData({  
        username: formData.username,  
        gender: formData.gender  
      });  
    }  
    wx.navigateBack(); // 返回前一个页面  
  }  
});
// Config for Mate Tracker ESP32
const CONFIG={
appName:"Mate Service",
appSubtitle:"Share, Track, and Enjoy Together",
appEmoji:"🧉",
terminology:{item:"Flavor",items:"Flavors",unit:"bottle",units:"bottles",category:"Category"},
emojis:{stock:"📦",consumed:"✅",remaining:"🧉",price:"💰",user:"👥",chart:"📊",payment:"💳",balance:"💰",lowStock:"🧉"},
defaults:{initialStock:24,lowStockThreshold:6,currency:"€",currencyPosition:"before"},
labels:{trackConsumption:"Track Consumption",userBalances:"User Balances",processPayment:"Process Payment",stockDetails:"Stock Details",manageItems:"Manage Flavors",manageUsers:"Manage Users"},
placeholders:{itemName:"Flavor name",pricePerUnit:"Price",stock:"Stock",userName:"User name",selectUser:"Select user",selectItem:"Select flavor",unitsTaken:"Units",paymentAmount:"Amount"}
};
function formatCurrency(a){const f=a.toFixed(2);return CONFIG.defaults.currencyPosition==="before"?`${CONFIG.defaults.currency}${f}`:`${f}${CONFIG.defaults.currency}`}

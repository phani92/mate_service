// Mate Tracker ESP32 App
let state={users:[],items:[],consumption:[],payments:[]};
document.addEventListener('DOMContentLoaded',()=>{applyConfig();fetchState();initEventListeners();checkDeviceStatus()});

function applyConfig(){
document.getElementById('appTitle').textContent=`${CONFIG.appEmoji} ${CONFIG.appName}`;
document.getElementById('appSubtitle').textContent=CONFIG.appSubtitle;
document.getElementById('stockIcon').textContent=CONFIG.emojis.stock;
document.getElementById('consumedIcon').textContent=CONFIG.emojis.consumed;
document.getElementById('remainingIcon').textContent=CONFIG.emojis.remaining;
document.getElementById('stockUnitLabel').textContent=CONFIG.terminology.units;
document.getElementById('consumedUnitLabel').textContent=CONFIG.terminology.units;
document.getElementById('remainingUnitLabel').textContent=CONFIG.terminology.units;
document.getElementById('trackConsumptionLabel').innerHTML=`${CONFIG.emojis.chart} ${CONFIG.labels.trackConsumption}`;
document.getElementById('userBalancesLabel').innerHTML=`${CONFIG.emojis.balance} ${CONFIG.labels.userBalances}`;
document.getElementById('processPaymentLabel').innerHTML=`${CONFIG.emojis.payment} ${CONFIG.labels.processPayment}`;
document.getElementById('stockDetailsLabel').innerHTML=`📈 ${CONFIG.labels.stockDetails}`;
document.getElementById('manageItemsLabel').innerHTML=`🎨 ${CONFIG.labels.manageItems}`;
document.getElementById('manageUsersLabel').innerHTML=`${CONFIG.emojis.user} ${CONFIG.labels.manageUsers}`;
document.getElementById('itemColumnHeader').textContent=CONFIG.terminology.item;
document.getElementById('unitsColumnHeader').textContent=CONFIG.terminology.units;
}

async function checkDeviceStatus(){
try{
const r=await fetch('/api/status');
if(r.ok){
const d=await r.json();
document.getElementById('statusText').textContent=`Connected • ${d.wifi.ip}`;
}else{document.getElementById('statusText').textContent='Connected';}
}catch(e){document.getElementById('statusText').textContent='Offline';}
setInterval(checkDeviceStatus,30000);
}

async function apiCall(endpoint,method='GET',body=null){
try{
const opts={method,headers:{'Content-Type':'application/json'}};
if(body)opts.body=JSON.stringify(body);
const r=await fetch(endpoint,opts);
const d=await r.json();
if(!r.ok)throw new Error(d.error||'Error');
return d;
}catch(e){alert(e.message);console.error('API:',e);return null;}
}

async function fetchState(){
const s=await apiCall('/api/state');
if(s){state=s;render();}
}

function initEventListeners(){
document.getElementById('addUserBtn').addEventListener('click',addUser);
document.getElementById('addItemBtn').addEventListener('click',addItem);
document.getElementById('recordConsumptionBtn').addEventListener('click',recordConsumption);
document.getElementById('processPaymentBtn').addEventListener('click',processPayment);
document.getElementById('userName').addEventListener('keypress',e=>{if(e.key==='Enter')addUser()});
document.getElementById('itemName').addEventListener('keypress',e=>{if(e.key==='Enter')addItem()});
}

async function addUser(){
const i=document.getElementById('userName');
const n=i.value.trim();
if(!n){alert('Enter name');return;}
const s=await apiCall('/api/users','POST',{name:n});
if(s){state=s;i.value='';render();}
}

async function removeUser(id){
if(!confirm('Remove user and all their data?'))return;
const s=await apiCall(`/api/users/${id}`,'DELETE');
if(s){state=s;render();}
}

async function addItem(){
const ni=document.getElementById('itemName');
const pi=document.getElementById('itemPrice');
const si=document.getElementById('itemStock');
const n=ni.value.trim();
const p=parseFloat(pi.value);
const st=parseInt(si.value)||CONFIG.defaults.initialStock;
if(!n){alert('Enter item name');return;}
if(!p||p<=0){alert('Enter valid price');return;}
const s=await apiCall('/api/items','POST',{name:n,price:p,stock:st});
if(s){state=s;ni.value='';pi.value='';si.value=CONFIG.defaults.initialStock;render();}
}

async function removeItem(id){
if(!confirm('Remove item and all data?'))return;
const s=await apiCall(`/api/items/${id}`,'DELETE');
if(s){state=s;render();}
}

async function updateItemStock(id){
const ns=prompt('New stock amount:');
if(ns===null)return;
const st=parseInt(ns);
if(isNaN(st)||st<0){alert('Invalid stock');return;}
const s=await apiCall(`/api/items/${id}/stock`,'PUT',{stock:st});
if(s){state=s;render();}
}

async function recordConsumption(){
const us=document.getElementById('consumptionUser');
const is=document.getElementById('consumptionItem');
const ai=document.getElementById('consumptionAmount');
const uid=us.value,iid=is.value,qty=parseInt(ai.value);
if(!uid||!iid){alert('Select user and item');return;}
if(!qty||qty<=0){alert('Enter valid quantity');return;}
const s=await apiCall('/api/consumption','POST',{userId:uid,itemId:iid,quantity:qty});
if(s){state=s;ai.value='1';render();}
}

async function processPayment(){
const us=document.getElementById('paymentUser');
const is=document.getElementById('paymentItem');
const ai=document.getElementById('paymentAmount');
const uid=us.value,iid=is.value,amt=parseFloat(ai.value);
if(!uid||!iid){alert('Select user and item');return;}
if(!amt||amt<=0){alert('Enter valid amount');return;}
const s=await apiCall('/api/payments','POST',{userId:uid,itemId:iid,amount:amt});
if(s){state=s;ai.value='';render();}
}

function getTotalConsumed(itemId=null){
return state.consumption.filter(c=>!itemId||c.itemId===itemId).reduce((s,c)=>s+c.quantity,0);
}

function getTotalPaid(uid,iid){
return state.payments.filter(p=>p.userId===uid&&p.itemId===iid).reduce((s,p)=>s+(p.unitsPaid||0),0);
}

function getUserConsumption(uid,iid){
return state.consumption.filter(c=>c.userId===uid&&c.itemId===iid).reduce((s,c)=>s+c.quantity,0);
}

function getRemainingStock(iid){
const item=state.items.find(i=>i.id===iid);
if(!item)return 0;
return item.initialStock-getTotalConsumed(iid);
}

function getTotalStock(){return state.items.reduce((s,i)=>s+i.initialStock,0)}
function getTotalRemaining(){return state.items.reduce((s,i)=>s+getRemainingStock(i.id),0)}

function render(){
renderOverview();renderItems();renderUsers();renderBalanceTable();renderStockDetails();updateSelects();
}

function renderOverview(){
document.getElementById('totalStock').textContent=getTotalStock();
document.getElementById('totalConsumed').textContent=getTotalConsumed();
document.getElementById('totalRemaining').textContent=getTotalRemaining();
}

function renderItems(){
const c=document.getElementById('itemsList');
if(state.items.length===0){c.innerHTML=`<p style="color:var(--text-tertiary);text-align:center;padding:1rem">No items yet</p>`;return;}
c.innerHTML=state.items.map(i=>{
const r=getRemainingStock(i.id);
return`<div class="item-card"><div class="item-info"><div class="item-name">${esc(i.name)}</div><div class="item-details"><span>${CONFIG.emojis.price} ${formatCurrency(i.price)}</span><span>${CONFIG.emojis.remaining} ${r} left</span></div></div><button class="btn btn-small btn-danger" onclick="removeItem('${i.id}')">Remove</button></div>`;
}).join('');
}

function renderUsers(){
const c=document.getElementById('usersList');
if(state.users.length===0){c.innerHTML='<p style="color:var(--text-tertiary);text-align:center;padding:1rem">No users yet</p>';return;}
c.innerHTML=state.users.map(u=>`<div class="user-item"><div class="user-info"><div class="user-name">${esc(u.name)}</div></div><button class="btn btn-small btn-danger" onclick="removeUser('${u.id}')">Remove</button></div>`).join('');
}

function renderBalanceTable(){
const tb=document.getElementById('balanceTableBody');
const bal=[];
state.users.forEach(u=>{
state.items.forEach(i=>{
const con=getUserConsumption(u.id,i.id);
const paid=getTotalPaid(u.id,i.id);
const owed=con-paid;
if(con>0||paid>0){bal.push({userName:u.name,userId:u.id,itemName:i.name,itemId:i.id,quantity:con,paid,owed,amountOwed:owed*i.price})}
})
});
if(bal.length===0){tb.innerHTML='<tr><td colspan="5" class="empty-state">No consumption yet</td></tr>';return;}
tb.innerHTML=bal.map(b=>`<tr><td>${esc(b.userName)}</td><td>${esc(b.itemName)}</td><td>${b.quantity}/${b.paid}</td><td style="color:${b.amountOwed>0?'var(--warning)':'var(--success)'};font-weight:600">${formatCurrency(Math.abs(b.amountOwed))} ${b.amountOwed>0?'owed':b.amountOwed<0?'credit':''}</td><td>${b.owed>0?`<button class="btn btn-small btn-accent" onclick="quickPay('${b.userId}','${b.itemId}',${b.amountOwed})">Pay</button>`:''}</td></tr>`).join('');
}

function renderStockDetails(){
const c=document.getElementById('stockDetails');
if(state.items.length===0){c.innerHTML=`<p style="color:var(--text-tertiary);text-align:center;padding:1rem">No items yet</p>`;return;}
c.innerHTML=state.items.map(i=>{
const con=getTotalConsumed(i.id);
const rem=getRemainingStock(i.id);
const pct=i.initialStock>0?(rem/i.initialStock)*100:0;
let vis=rem<=CONFIG.defaults.lowStockThreshold&&rem>0?`<div class="stock-icons">${CONFIG.emojis.lowStock.repeat(rem)} ${rem}</div>`:rem>CONFIG.defaults.lowStockThreshold?`<div class="stock-count">${rem} left</div>`:'<div class="stock-count" style="color:var(--danger)">Out of stock</div>';
return`<div class="stock-item"><div class="stock-header"><div class="stock-item-name">${esc(i.name)}</div><button class="btn btn-small btn-primary" onclick="updateItemStock('${i.id}')">Update</button></div><div class="stock-info-row"><span class="stock-price">${CONFIG.emojis.price} ${formatCurrency(i.price)}/${CONFIG.terminology.unit}</span>${vis}</div><div class="progress-bar"><div class="progress-fill" style="width:${pct}%"></div></div></div>`;
}).join('');
}

function updateSelects(){
const uSels=[document.getElementById('consumptionUser'),document.getElementById('paymentUser')];
uSels.forEach(s=>{const v=s.value;s.innerHTML=`<option value="">${CONFIG.placeholders.selectUser}</option>`+state.users.map(u=>`<option value="${u.id}">${esc(u.name)}</option>`).join('');s.value=v});
const iSels=[document.getElementById('consumptionItem'),document.getElementById('paymentItem')];
iSels.forEach(s=>{const v=s.value;s.innerHTML=`<option value="">${CONFIG.placeholders.selectItem}</option>`+state.items.map(i=>`<option value="${i.id}">${esc(i.name)}</option>`).join('');s.value=v});
}

function quickPay(uid,iid,amt){
document.getElementById('paymentUser').value=uid;
document.getElementById('paymentItem').value=iid;
document.getElementById('paymentAmount').value=amt.toFixed(2);
document.querySelector('#processPaymentBtn').scrollIntoView({behavior:'smooth',block:'center'});
}

function esc(t){const d=document.createElement('div');d.textContent=t;return d.innerHTML}

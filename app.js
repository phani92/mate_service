// Application State
let state = {
    users: [],
    items: [],
    consumption: [], // { userId, itemId, quantity, timestamp }
    payments: [] // { userId, itemId, amount, timestamp }
};

// Initialize app
document.addEventListener('DOMContentLoaded', () => {
    applyConfig();
    fetchState();
    initEventListeners();
});

// Apply configuration to UI
function applyConfig() {
    // App branding
    document.getElementById('appTitle').textContent = `${CONFIG.appEmoji} ${CONFIG.appName}`;
    document.getElementById('appSubtitle').textContent = CONFIG.appSubtitle;
    document.title = `${CONFIG.appName} - Track & Manage Shared ${CONFIG.terminology.items}`;

    // Icons
    document.getElementById('stockIcon').textContent = CONFIG.emojis.stock;
    document.getElementById('consumedIcon').textContent = CONFIG.emojis.consumed;
    document.getElementById('remainingIcon').textContent = CONFIG.emojis.remaining;

    // Unit labels
    document.getElementById('stockUnitLabel').textContent = CONFIG.terminology.units;
    document.getElementById('consumedUnitLabel').textContent = CONFIG.terminology.units;
    document.getElementById('remainingUnitLabel').textContent = CONFIG.terminology.units;

    // Section labels
    document.getElementById('trackConsumptionLabel').innerHTML = `${CONFIG.emojis.chart} ${CONFIG.labels.trackConsumption}`;
    document.getElementById('userBalancesLabel').innerHTML = `${CONFIG.emojis.balance} ${CONFIG.labels.userBalances}`;
    document.getElementById('processPaymentLabel').innerHTML = `${CONFIG.emojis.payment} ${CONFIG.labels.processPayment}`;
    document.getElementById('stockDetailsLabel').innerHTML = `📈 ${CONFIG.labels.stockDetails}`;
    document.getElementById('manageItemsLabel').innerHTML = `🎨 ${CONFIG.labels.manageItems}`;
    document.getElementById('manageUsersLabel').innerHTML = `${CONFIG.emojis.user} ${CONFIG.labels.manageUsers}`;

    // Table headers
    document.getElementById('itemColumnHeader').textContent = CONFIG.terminology.item;
    document.getElementById('unitsColumnHeader').textContent = CONFIG.terminology.units;

    // Placeholders
    document.getElementById('itemName').placeholder = CONFIG.placeholders.itemName;
    document.getElementById('itemPrice').placeholder = CONFIG.placeholders.pricePerUnit;
    document.getElementById('itemStock').placeholder = `${CONFIG.placeholders.stock} (default ${CONFIG.defaults.initialStock})`;
    document.getElementById('itemStock').value = CONFIG.defaults.initialStock;
    document.getElementById('userName').placeholder = CONFIG.placeholders.userName;
    document.getElementById('consumptionAmount').placeholder = CONFIG.placeholders.unitsTaken;
    document.getElementById('paymentAmount').placeholder = CONFIG.placeholders.paymentAmount;
}

// API Helper
async function apiCall(endpoint, method = 'GET', body = null) {
    try {
        const options = {
            method,
            headers: {
                'Content-Type': 'application/json'
            }
        };
        if (body) options.body = JSON.stringify(body);

        const response = await fetch(endpoint, options);
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || 'Something went wrong');
        }

        return data;
    } catch (error) {
        alert(error.message);
        console.error('API Error:', error);
        return null;
    }
}

async function fetchState() {
    const newState = await apiCall('/api/state');
    if (newState) {
        state = newState;
        render();
    }
}

// Event Listeners
function initEventListeners() {
    document.getElementById('addUserBtn').addEventListener('click', addUser);
    document.getElementById('addItemBtn').addEventListener('click', addItem);
    document.getElementById('recordConsumptionBtn').addEventListener('click', recordConsumption);
    document.getElementById('processPaymentBtn').addEventListener('click', processPayment);

    // Enter key support
    document.getElementById('userName').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') addUser();
    });
    document.getElementById('itemName').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') addItem();
    });
}

// User Management
async function addUser() {
    const nameInput = document.getElementById('userName');
    const name = nameInput.value.trim();

    if (!name) {
        alert('Please enter a name');
        return;
    }

    const newState = await apiCall('/api/users', 'POST', { name });
    if (newState) {
        state = newState;
        nameInput.value = '';
        render();
    }
}

async function removeUser(userId) {
    if (!confirm('Are you sure you want to remove this user? All their consumption data will be deleted.')) {
        return;
    }

    const newState = await apiCall(`/api/users/${userId}`, 'DELETE');
    if (newState) {
        state = newState;
        render();
    }
}

// Item Management
async function addItem() {
    const nameInput = document.getElementById('itemName');
    const priceInput = document.getElementById('itemPrice');
    const stockInput = document.getElementById('itemStock');

    const name = nameInput.value.trim();
    const price = parseFloat(priceInput.value);
    const stock = parseInt(stockInput.value) || CONFIG.defaults.initialStock;

    if (!name) {
        alert(`Please enter a ${CONFIG.terminology.item.toLowerCase()} name`);
        return;
    }

    if (!price || price <= 0) {
        alert('Please enter a valid price');
        return;
    }

    const newState = await apiCall('/api/items', 'POST', { name, price, stock });
    if (newState) {
        state = newState;
        nameInput.value = '';
        priceInput.value = '';
        stockInput.value = CONFIG.defaults.initialStock;
        render();
    }
}

async function removeItem(itemId) {
    if (!confirm(`Are you sure you want to remove this ${CONFIG.terminology.item.toLowerCase()}? All consumption data for this ${CONFIG.terminology.item.toLowerCase()} will be deleted.`)) {
        return;
    }

    const newState = await apiCall(`/api/items/${itemId}`, 'DELETE');
    if (newState) {
        state = newState;
        render();
    }
}

async function updateItemStock(itemId) {
    const newStock = prompt('Enter new stock amount:');
    if (newStock === null) return;

    const stock = parseInt(newStock);
    if (isNaN(stock) || stock < 0) {
        alert('Please enter a valid stock amount');
        return;
    }

    const newState = await apiCall(`/api/items/${itemId}/stock`, 'PUT', { stock });
    if (newState) {
        state = newState;
        render();
    }
}

// Consumption Tracking
async function recordConsumption() {
    const userSelect = document.getElementById('consumptionUser');
    const itemSelect = document.getElementById('consumptionItem');
    const amountInput = document.getElementById('consumptionAmount');

    const userId = userSelect.value;
    const itemId = itemSelect.value;
    const quantity = parseInt(amountInput.value);

    if (!userId || !itemId) {
        alert(`Please select both user and ${CONFIG.terminology.item.toLowerCase()}`);
        return;
    }

    if (!quantity || quantity <= 0) {
        alert(`Please enter a valid number of ${CONFIG.terminology.units}`);
        return;
    }

    const newState = await apiCall('/api/consumption', 'POST', { userId, itemId, quantity });
    if (newState) {
        state = newState;
        amountInput.value = '1';
        render();
    }
}

// Payment Processing
async function processPayment() {
    const userSelect = document.getElementById('paymentUser');
    const itemSelect = document.getElementById('paymentItem');
    const amountInput = document.getElementById('paymentAmount');

    const userId = userSelect.value;
    const itemId = itemSelect.value;
    const amount = parseFloat(amountInput.value);

    if (!userId || !itemId) {
        alert(`Please select both user and ${CONFIG.terminology.item.toLowerCase()}`);
        return;
    }

    if (!amount || amount <= 0) {
        alert('Please enter a valid payment amount');
        return;
    }

    const newState = await apiCall('/api/payments', 'POST', { userId, itemId, amount });
    if (newState) {
        state = newState;
        amountInput.value = '';
        render();
    }
}

// Calculations
function getTotalConsumed(itemId = null) {
    return state.consumption
        .filter(c => !itemId || c.itemId === itemId)
        .reduce((sum, c) => sum + c.quantity, 0);
}

function getTotalPaid(userId, itemId) {
    return state.payments
        .filter(p => p.userId === userId && p.itemId === itemId)
        .reduce((sum, p) => sum + p.unitsPaid, 0);
}

function getUserConsumption(userId, itemId) {
    return state.consumption
        .filter(c => c.userId === userId && c.itemId === itemId)
        .reduce((sum, c) => sum + c.quantity, 0);
}

function getRemainingStock(itemId) {
    const item = state.items.find(i => i.id === itemId);
    if (!item) return 0;

    const consumed = getTotalConsumed(itemId);
    return item.initialStock - consumed;
}

function getTotalStock() {
    return state.items.reduce((sum, i) => sum + i.initialStock, 0);
}

function getTotalRemaining() {
    return state.items.reduce((sum, i) => sum + getRemainingStock(i.id), 0);
}

// Rendering
function render() {
    renderOverview();
    renderItems();
    renderUsers();
    renderBalanceTable();
    renderStockDetails();
    updateSelects();
}

function renderOverview() {
    const totalStock = getTotalStock();
    const totalConsumed = getTotalConsumed();
    const totalRemaining = getTotalRemaining();

    document.getElementById('totalStock').textContent = totalStock;
    document.getElementById('totalConsumed').textContent = totalConsumed;
    document.getElementById('totalRemaining').textContent = totalRemaining;
}

function renderItems() {
    const container = document.getElementById('itemsList');

    if (state.items.length === 0) {
        container.innerHTML = `<p style="color: var(--text-tertiary); text-align: center; padding: 1rem;">No ${CONFIG.terminology.items.toLowerCase()} added yet</p>`;
        return;
    }

    container.innerHTML = state.items.map(item => {
        const remaining = getRemainingStock(item.id);

        return `
            <div class="item-card">
                <div class="item-info">
                    <div class="item-name">${escapeHtml(item.name)}</div>
                    <div class="item-details">
                        <span>${CONFIG.emojis.price} ${formatCurrency(item.price)}/${CONFIG.terminology.unit}</span>
                        <span>${CONFIG.emojis.remaining} ${remaining} left</span>
                    </div>
                </div>
                <div style="display: flex; gap: 0.5rem;">
                    <button class="btn btn-small btn-danger" onclick="removeItem('${item.id}')">Remove</button>
                </div>
            </div>
        `;
    }).join('');
}

function renderUsers() {
    const container = document.getElementById('usersList');

    if (state.users.length === 0) {
        container.innerHTML = '<p style="color: var(--text-tertiary); text-align: center; padding: 1rem;">No users added yet</p>';
        return;
    }

    container.innerHTML = state.users.map(user => `
        <div class="user-item">
            <div class="user-info">
                <div class="user-name">${escapeHtml(user.name)}</div>
            </div>
            <button class="btn btn-small btn-danger" onclick="removeUser('${user.id}')">Remove</button>
        </div>
    `).join('');
}

function renderBalanceTable() {
    const tbody = document.getElementById('balanceTableBody');

    // Create a map of user-item combinations with balances
    const balances = [];

    state.users.forEach(user => {
        state.items.forEach(item => {
            const consumed = getUserConsumption(user.id, item.id);
            const paid = getTotalPaid(user.id, item.id);
            const owed = consumed - paid;

            if (consumed > 0 || paid > 0) {
                balances.push({
                    userName: user.name,
                    userId: user.id,
                    itemName: item.name,
                    itemId: item.id,
                    quantity: consumed,
                    paid: paid,
                    owed: owed,
                    amountOwed: owed * item.price
                });
            }
        });
    });

    if (balances.length === 0) {
        tbody.innerHTML = '<tr><td colspan="5" class="empty-state">No consumption recorded yet</td></tr>';
        return;
    }

    tbody.innerHTML = balances.map(b => `
        <tr>
            <td>${escapeHtml(b.userName)}</td>
            <td>${escapeHtml(b.itemName)}</td>
            <td>${b.quantity} consumed / ${b.paid} paid</td>
            <td style="color: ${b.amountOwed > 0 ? 'var(--warning)' : 'var(--success)'}; font-weight: 600;">
                ${formatCurrency(Math.abs(b.amountOwed))} ${b.amountOwed > 0 ? 'owed' : b.amountOwed < 0 ? 'credit' : ''}
            </td>
            <td>
                ${b.owed > 0 ? `<button class="btn btn-small btn-accent" onclick="quickPayment('${b.userId}', '${b.itemId}', ${b.amountOwed})">Pay Now</button>` : ''}
            </td>
        </tr>
    `).join('');
}

function renderStockDetails() {
    const container = document.getElementById('stockDetails');

    if (state.items.length === 0) {
        container.innerHTML = `<p style="color: var(--text-tertiary); text-align: center; padding: 1rem;">No ${CONFIG.terminology.items.toLowerCase()} added yet</p>`;
        return;
    }

    container.innerHTML = state.items.map(item => {
        const consumed = getTotalConsumed(item.id);
        const remaining = getRemainingStock(item.id);
        const percentage = item.initialStock > 0 ? (remaining / item.initialStock) * 100 : 0;
        
        // Generate icons for stock at or below threshold
        let stockVisualization = '';
        if (remaining <= CONFIG.defaults.lowStockThreshold && remaining > 0) {
            stockVisualization = `<div class="stock-icons">${CONFIG.emojis.lowStock.repeat(remaining)} ${remaining} left</div>`;
        } else if (remaining > CONFIG.defaults.lowStockThreshold) {
            stockVisualization = `<div class="stock-count">${remaining} ${CONFIG.terminology.units} left</div>`;
        } else {
            stockVisualization = '<div class="stock-count" style="color: var(--danger);">Out of stock</div>';
        }

        return `
            <div class="stock-item">
                <div class="stock-header">
                    <div class="stock-item-name">${escapeHtml(item.name)}</div>
                    <button class="btn btn-small btn-primary" onclick="updateItemStock('${item.id}')">Update Stock</button>
                </div>
                <div class="stock-info-row">
                    <span class="stock-price">${CONFIG.emojis.price} ${formatCurrency(item.price)}/${CONFIG.terminology.unit}</span>
                    ${stockVisualization}
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: ${percentage}%"></div>
                </div>
            </div>
        `;
    }).join('');
}

function updateSelects() {
    // Update user selects
    const userSelects = [
        document.getElementById('consumptionUser'),
        document.getElementById('paymentUser')
    ];

    userSelects.forEach(select => {
        const currentValue = select.value;
        select.innerHTML = `<option value="">${CONFIG.placeholders.selectUser}</option>` +
            state.users.map(u => `<option value="${u.id}">${escapeHtml(u.name)}</option>`).join('');
        select.value = currentValue;
    });

    // Update item selects
    const itemSelects = [
        document.getElementById('consumptionItem'),
        document.getElementById('paymentItem')
    ];

    itemSelects.forEach(select => {
        const currentValue = select.value;
        select.innerHTML = `<option value="">${CONFIG.placeholders.selectItem}</option>` +
            state.items.map(i => `<option value="${i.id}">${escapeHtml(i.name)}</option>`).join('');
        select.value = currentValue;
    });
}

// Quick payment helper
function quickPayment(userId, itemId, amount) {
    document.getElementById('paymentUser').value = userId;
    document.getElementById('paymentItem').value = itemId;
    document.getElementById('paymentAmount').value = amount.toFixed(2);

    // Scroll to payment section
    document.querySelector('#processPaymentBtn').scrollIntoView({ behavior: 'smooth', block: 'center' });
}

// Utility
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

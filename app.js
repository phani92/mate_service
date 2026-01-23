// Constants
const LOW_STOCK_THRESHOLD = 6; // Show bottle icons when stock is at or below this level

// Application State
let state = {
    users: [],
    flavors: [],
    consumption: [], // { userId, flavorId, bottles, timestamp }
    payments: [] // { userId, flavorId, amount, timestamp }
};

// Initialize app
document.addEventListener('DOMContentLoaded', () => {
    fetchState();
    initEventListeners();
});

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
    document.getElementById('addFlavorBtn').addEventListener('click', addFlavor);
    document.getElementById('recordConsumptionBtn').addEventListener('click', recordConsumption);
    document.getElementById('processPaymentBtn').addEventListener('click', processPayment);

    // Enter key support
    document.getElementById('userName').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') addUser();
    });
    document.getElementById('flavorName').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') addFlavor();
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

// Flavor Management
async function addFlavor() {
    const nameInput = document.getElementById('flavorName');
    const priceInput = document.getElementById('flavorPrice');
    const stockInput = document.getElementById('flavorStock');

    const name = nameInput.value.trim();
    const price = parseFloat(priceInput.value);
    const stock = parseInt(stockInput.value) || 24;

    if (!name) {
        alert('Please enter a flavor name');
        return;
    }

    if (!price || price <= 0) {
        alert('Please enter a valid price');
        return;
    }

    const newState = await apiCall('/api/flavors', 'POST', { name, price, stock });
    if (newState) {
        state = newState;
        nameInput.value = '';
        priceInput.value = '';
        stockInput.value = '24';
        render();
    }
}

async function removeFlavor(flavorId) {
    if (!confirm('Are you sure you want to remove this flavor? All consumption data for this flavor will be deleted.')) {
        return;
    }

    const newState = await apiCall(`/api/flavors/${flavorId}`, 'DELETE');
    if (newState) {
        state = newState;
        render();
    }
}

async function updateFlavorStock(flavorId) {
    const newStock = prompt('Enter new stock amount:');
    if (newStock === null) return;

    const stock = parseInt(newStock);
    if (isNaN(stock) || stock < 0) {
        alert('Please enter a valid stock amount');
        return;
    }

    const newState = await apiCall(`/api/flavors/${flavorId}/stock`, 'PUT', { stock });
    if (newState) {
        state = newState;
        render();
    }
}

// Consumption Tracking
async function recordConsumption() {
    const userSelect = document.getElementById('consumptionUser');
    const flavorSelect = document.getElementById('consumptionFlavor');
    const amountInput = document.getElementById('consumptionAmount');

    const userId = userSelect.value;
    const flavorId = flavorSelect.value;
    const bottles = parseInt(amountInput.value);

    if (!userId || !flavorId) {
        alert('Please select both user and flavor');
        return;
    }

    if (!bottles || bottles <= 0) {
        alert('Please enter a valid number of bottles');
        return;
    }

    const newState = await apiCall('/api/consumption', 'POST', { userId, flavorId, bottles });
    if (newState) {
        state = newState;
        amountInput.value = '1';
        render();
    }
}

// Payment Processing
async function processPayment() {
    const userSelect = document.getElementById('paymentUser');
    const flavorSelect = document.getElementById('paymentFlavor');
    const amountInput = document.getElementById('paymentAmount');

    const userId = userSelect.value;
    const flavorId = flavorSelect.value;
    const amount = parseFloat(amountInput.value);

    if (!userId || !flavorId) {
        alert('Please select both user and flavor');
        return;
    }

    if (!amount || amount <= 0) {
        alert('Please enter a valid payment amount');
        return;
    }

    const newState = await apiCall('/api/payments', 'POST', { userId, flavorId, amount });
    if (newState) {
        state = newState;
        amountInput.value = '';
        render();
    }
}

// Calculations
function getTotalConsumed(flavorId = null) {
    return state.consumption
        .filter(c => !flavorId || c.flavorId === flavorId)
        .reduce((sum, c) => sum + c.bottles, 0);
}

function getTotalPaid(userId, flavorId) {
    return state.payments
        .filter(p => p.userId === userId && p.flavorId === flavorId)
        .reduce((sum, p) => sum + p.bottlesPaid, 0);
}

function getUserConsumption(userId, flavorId) {
    return state.consumption
        .filter(c => c.userId === userId && c.flavorId === flavorId)
        .reduce((sum, c) => sum + c.bottles, 0);
}

function getRemainingStock(flavorId) {
    const flavor = state.flavors.find(f => f.id === flavorId);
    if (!flavor) return 0;

    const consumed = getTotalConsumed(flavorId);
    return flavor.initialStock - consumed;
}

function getTotalStock() {
    return state.flavors.reduce((sum, f) => sum + f.initialStock, 0);
}

function getTotalRemaining() {
    return state.flavors.reduce((sum, f) => sum + getRemainingStock(f.id), 0);
}

// Rendering
function render() {
    renderOverview();
    renderFlavors();
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

function renderFlavors() {
    const container = document.getElementById('flavorsList');

    if (state.flavors.length === 0) {
        container.innerHTML = '<p style="color: var(--text-tertiary); text-align: center; padding: 1rem;">No flavors added yet</p>';
        return;
    }

    container.innerHTML = state.flavors.map(flavor => {
        const remaining = getRemainingStock(flavor.id);
        const consumed = getTotalConsumed(flavor.id);

        return `
            <div class="flavor-item">
                <div class="flavor-info">
                    <div class="flavor-name">${escapeHtml(flavor.name)}</div>
                    <div class="flavor-details">
                        <span>💰 €${flavor.price.toFixed(2)}/bottle</span>
                        <span>🍾 ${remaining} left</span>
                    </div>
                </div>
                <div style="display: flex; gap: 0.5rem;">
                    <button class="btn btn-small btn-danger" onclick="removeFlavor('${flavor.id}')">Remove</button>
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

    // Create a map of user-flavor combinations with balances
    const balances = [];

    state.users.forEach(user => {
        state.flavors.forEach(flavor => {
            const consumed = getUserConsumption(user.id, flavor.id);
            const paid = getTotalPaid(user.id, flavor.id);
            const owed = consumed - paid;

            if (consumed > 0 || paid > 0) {
                balances.push({
                    userName: user.name,
                    userId: user.id,
                    flavorName: flavor.name,
                    flavorId: flavor.id,
                    bottles: consumed,
                    paid: paid,
                    owed: owed,
                    amountOwed: owed * flavor.price
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
            <td>${escapeHtml(b.flavorName)}</td>
            <td>${b.bottles} consumed / ${b.paid} paid</td>
            <td style="color: ${b.amountOwed > 0 ? 'var(--warning)' : 'var(--success)'}; font-weight: 600;">
                €${Math.abs(b.amountOwed).toFixed(2)} ${b.amountOwed > 0 ? 'owed' : b.amountOwed < 0 ? 'credit' : ''}
            </td>
            <td>
                ${b.owed > 0 ? `<button class="btn btn-small btn-accent" onclick="quickPayment('${b.userId}', '${b.flavorId}', ${b.amountOwed})">Pay Now</button>` : ''}
            </td>
        </tr>
    `).join('');
}

function renderStockDetails() {
    const container = document.getElementById('stockDetails');

    if (state.flavors.length === 0) {
        container.innerHTML = '<p style="color: var(--text-tertiary); text-align: center; padding: 1rem;">No flavors added yet</p>';
        return;
    }

    container.innerHTML = state.flavors.map(flavor => {
        const consumed = getTotalConsumed(flavor.id);
        const remaining = getRemainingStock(flavor.id);
        const percentage = flavor.initialStock > 0 ? (remaining / flavor.initialStock) * 100 : 0;
        
        // Generate bottle icons for stock at or below threshold
        let stockVisualization = '';
        if (remaining <= LOW_STOCK_THRESHOLD && remaining > 0) {
            stockVisualization = `<div class="bottle-icons">${'🧉'.repeat(remaining)} ${remaining} left</div>`;
        } else if (remaining > LOW_STOCK_THRESHOLD) {
            stockVisualization = `<div class="stock-count">${remaining} bottles left</div>`;
        } else {
            stockVisualization = '<div class="stock-count" style="color: var(--danger);">Out of stock</div>';
        }

        return `
            <div class="stock-item">
                <div class="stock-header">
                    <div class="stock-flavor-name">${escapeHtml(flavor.name)}</div>
                    <button class="btn btn-small btn-primary" onclick="updateFlavorStock('${flavor.id}')">Update Stock</button>
                </div>
                <div class="stock-info-row">
                    <span class="stock-price">💰 €${flavor.price.toFixed(2)}/bottle</span>
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
        select.innerHTML = '<option value="">Select user</option>' +
            state.users.map(u => `<option value="${u.id}">${escapeHtml(u.name)}</option>`).join('');
        select.value = currentValue;
    });

    // Update flavor selects
    const flavorSelects = [
        document.getElementById('consumptionFlavor'),
        document.getElementById('paymentFlavor')
    ];

    flavorSelects.forEach(select => {
        const currentValue = select.value;
        select.innerHTML = '<option value="">Select flavor</option>' +
            state.flavors.map(f => `<option value="${f.id}">${escapeHtml(f.name)}</option>`).join('');
        select.value = currentValue;
    });
}

// Quick payment helper
function quickPayment(userId, flavorId, amount) {
    document.getElementById('paymentUser').value = userId;
    document.getElementById('paymentFlavor').value = flavorId;
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

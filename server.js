const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;
const DATA_FILE = path.join(__dirname, 'mate_data.json');

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(express.static(__dirname)); // Serve static files from current directory

// Initial State
let state = {
    users: [],
    flavors: [],
    consumption: [],
    payments: []
};

// Load state from file
function loadState() {
    if (fs.existsSync(DATA_FILE)) {
        try {
            const data = fs.readFileSync(DATA_FILE, 'utf8');
            state = JSON.parse(data);
            console.log('State loaded from file');
        } catch (err) {
            console.error('Error loading state:', err);
        }
    }
}

// Save state to file
function saveState() {
    try {
        fs.writeFileSync(DATA_FILE, JSON.stringify(state, null, 2));
        console.log('State saved to file');
    } catch (err) {
        console.error('Error saving state:', err);
    }
}

// Initialize
loadState();

// Routes

// Get full state
app.get('/api/state', (req, res) => {
    res.json(state);
});

// Add User
app.post('/api/users', (req, res) => {
    const { name } = req.body;
    if (!name) return res.status(400).json({ error: 'Name is required' });

    if (state.users.find(u => u.name.toLowerCase() === name.toLowerCase())) {
        return res.status(400).json({ error: 'User already exists' });
    }

    const newUser = {
        id: Date.now().toString(),
        name
    };
    state.users.push(newUser);
    saveState();
    res.json(state);
});

// Remove User
app.delete('/api/users/:id', (req, res) => {
    const { id } = req.params;
    state.users = state.users.filter(u => u.id !== id);
    state.consumption = state.consumption.filter(c => c.userId !== id);
    state.payments = state.payments.filter(p => p.userId !== id);
    saveState();
    res.json(state);
});

// Add Flavor
app.post('/api/flavors', (req, res) => {
    const { name, price, stock } = req.body;
    if (!name || !price) return res.status(400).json({ error: 'Invalid input' });

    if (state.flavors.find(f => f.name.toLowerCase() === name.toLowerCase())) {
        return res.status(400).json({ error: 'Flavor already exists' });
    }

    const newFlavor = {
        id: Date.now().toString(),
        name,
        price: parseFloat(price),
        initialStock: parseInt(stock) || 0
    };
    state.flavors.push(newFlavor);
    saveState();
    res.json(state);
});

// Remove Flavor
app.delete('/api/flavors/:id', (req, res) => {
    const { id } = req.params;
    state.flavors = state.flavors.filter(f => f.id !== id);
    state.consumption = state.consumption.filter(c => c.flavorId !== id);
    state.payments = state.payments.filter(p => p.flavorId !== id);
    saveState();
    res.json(state);
});

// Update Flavor Stock
app.put('/api/flavors/:id/stock', (req, res) => {
    const { id } = req.params;
    const { stock } = req.body;

    const flavor = state.flavors.find(f => f.id === id);
    if (!flavor) return res.status(404).json({ error: 'Flavor not found' });

    flavor.initialStock = parseInt(stock);
    saveState();
    res.json(state);
});

// Record Consumption
app.post('/api/consumption', (req, res) => {
    const { userId, flavorId, bottles } = req.body;

    if (!userId || !flavorId || !bottles) {
        return res.status(400).json({ error: 'Invalid input' });
    }

    // Check stock (optional, but good practice)
    // We'll skip strict server-side stock check for now to keep it simple, 
    // or we can duplicate the logic. Let's duplicate for safety.
    const flavor = state.flavors.find(f => f.id === flavorId);
    if (!flavor) return res.status(404).json({ error: 'Flavor not found' });

    const consumed = state.consumption
        .filter(c => c.flavorId === flavorId)
        .reduce((sum, c) => sum + c.bottles, 0);

    const remaining = flavor.initialStock - consumed;
    if (bottles > remaining) {
        return res.status(400).json({ error: 'Not enough stock' });
    }

    state.consumption.push({
        id: Date.now().toString(),
        userId,
        flavorId,
        bottles: parseInt(bottles),
        timestamp: new Date().toISOString()
    });
    saveState();
    res.json(state);
});

// Process Payment
app.post('/api/payments', (req, res) => {
    const { userId, flavorId, amount } = req.body;

    if (!userId || !flavorId || !amount) {
        return res.status(400).json({ error: 'Invalid input' });
    }

    const flavor = state.flavors.find(f => f.id === flavorId);
    if (!flavor) return res.status(404).json({ error: 'Flavor not found' });

    const bottlesPaid = Math.floor(parseFloat(amount) / flavor.price);

    state.payments.push({
        id: Date.now().toString(),
        userId,
        flavorId,
        amount: parseFloat(amount),
        bottlesPaid,
        timestamp: new Date().toISOString()
    });
    saveState();
    res.json(state);
});

// Start server
// Start server
app.listen(PORT, '0.0.0.0', () => {
    console.log(`Server running on http://localhost:${PORT}`);

    // Log local IP addresses to help user connect from other machines
    const os = require('os');
    const interfaces = os.networkInterfaces();
    console.log('Available on your network at:');
    Object.keys(interfaces).forEach((ifname) => {
        interfaces[ifname].forEach((iface) => {
            if ('IPv4' !== iface.family || iface.internal !== false) {
                return;
            }
            console.log(`  http://${iface.address}:${PORT}`);
        });
    });
});

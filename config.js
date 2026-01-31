// ============================================
// INVENTORY CONFIGURATION
// ============================================
// Customize this file to adapt the app for different inventory types
// Examples: Mate drinks, Chips, Snacks, Coffee, Office supplies, etc.

const CONFIG = {
    // App branding
    appName: "Mate Service",
    appSubtitle: "Share, Track, and Enjoy Together",
    appEmoji: "🧉",  // Main app emoji (e.g., 🧉 for mate, 🥤 for drinks, 🍿 for snacks)

    // Item terminology - customize these for your inventory type
    terminology: {
        item: "Flavor",         // Singular (e.g., "Flavor", "Product", "Snack")
        items: "Flavors",       // Plural
        unit: "bottle",         // Singular unit (e.g., "bottle", "bag", "piece")
        units: "bottles",       // Plural units
        category: "Category",   // Category label (e.g., "Flavor", "Type", "Brand")
        categories: "Categories"
    },

    // Emojis used throughout the app
    emojis: {
        stock: "📦",            // Total stock icon
        consumed: "✅",         // Consumed icon
        remaining: "🧉",        // Remaining icon (e.g., 🧉, 🥤, 🍿)
        price: "💰",            // Price icon
        user: "👥",             // Users icon
        chart: "📊",            // Chart/tracking icon
        payment: "💳",          // Payment icon
        balance: "💰",          // Balance icon
        lowStock: "🧉"          // Low stock visual (repeated for each unit)
    },

    // Default values
    defaults: {
        initialStock: 24,       // Default stock when adding new item
        lowStockThreshold: 6,   // Show visual icons when stock is at or below this
        currency: "€",          // Currency symbol
        currencyPosition: "before" // "before" (€10) or "after" (10€)
    },

    // UI Labels - customize section headings
    labels: {
        trackConsumption: "Track Consumption",
        userBalances: "User Balances",
        processPayment: "Process Payment",
        stockDetails: "Stock Details",
        manageItems: "Manage Flavors",
        manageUsers: "Manage Users"
    },

    // Placeholder text
    placeholders: {
        itemName: "Flavor name (e.g., Original, Lemon)",
        pricePerUnit: "Price per bottle",
        stock: "Stock",
        userName: "Enter colleague's name",
        selectUser: "Select user",
        selectItem: "Select flavor",
        unitsTaken: "Bottles taken",
        paymentAmount: "Payment amount"
    }
};

// Format currency based on config
function formatCurrency(amount) {
    const formatted = amount.toFixed(2);
    return CONFIG.defaults.currencyPosition === "before" 
        ? `${CONFIG.defaults.currency}${formatted}`
        : `${formatted}${CONFIG.defaults.currency}`;
}

// Export for use in other files (works in both browser and Node.js)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { CONFIG, formatCurrency };
}

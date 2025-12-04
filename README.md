# Mate Service 🧉

A simple, self-hosted web application to track Mate consumption and payments among colleagues or friends.

## Features

-   **User Management**: Add and remove users easily.
-   **Flavor Tracking**: Manage different Mate flavors, prices, and stock levels.
-   **Consumption Log**: Track who drank what and when.
-   **Balance System**: Automatically calculates how much each user owes based on their consumption.
-   **Payments**: Record payments to settle balances.
-   **Network Access**: Host on one machine and access from any device on the same network.
-   **Simple Storage**: Data is stored in a simple local JSON file (`mate_data.json`).

## Prerequisites

-   [Node.js](https://nodejs.org/) (v14 or higher recommended)
-   npm (usually comes with Node.js)

## Installation

1.  Clone the repository or download the source code.
2.  Navigate to the project directory:
    ```bash
    cd mate_service
    ```
3.  Install dependencies:
    ```bash
    npm install
    ```

## Usage

### Starting the Server

Run the following command to start the server:

```bash
node server.js
```

The server will start and display the access URLs:

```text
Server running on http://localhost:3000
Available on your network at:
  http://192.168.1.X:3000
```

### Accessing the App

-   **On the host machine**: Open `http://localhost:3000` in your browser.
-   **On other devices**: Use the network URL provided in the terminal output (e.g., `http://192.168.1.X:3000`).

## Running on Windows

The application is fully portable. To run it on Windows:

1.  Ensure **Node.js** is installed.
2.  Double-click the `start_server.bat` file included in the folder.
    -   This script will automatically install dependencies (if needed) and start the server.
    -   It will keep the window open so you can see the network URL to share with others.

### Data Storage

All data is automatically saved to `mate_data.json` in the project directory. You can back up this file to preserve your data.

## License

ISC

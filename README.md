# BusNova — C-Powered Bus Reservation System

A local prototype with a navy-blue/orange passenger interface and an admin management interface, sharing the same C backend and binary file records.

## Run on Windows 11 (MSYS2 UCRT64)

1. Extract the ZIP into `C:\Users\Asus\Downloads\BusNova_Upgraded` (or any folder).
2. Open **MSYS2 UCRT64**.
3. Run:

```bash
cd /c/Users/Asus/Downloads/BusNova_Upgraded
gcc server.c -o busnova.exe -lws2_32
./busnova.exe
```

4. Open **http://127.0.0.1:8080/** for passengers.
5. Open **http://127.0.0.1:8080/admin** for administration.
6. Keep the terminal open. Press Ctrl+C to stop the server.

### Existing data

`buses.dat` and `bookings.dat` are generated in the current working directory. To migrate existing records, close both servers and copy your previous `.dat` files into this folder. Back up your files first. Do not upload real passenger records to GitHub.

## Features

- Passenger route/date search, live seat availability, seat selection, booking, and ticket lookup by booking ID + phone.
- Admin bus and reservation CRUD, search, statistics, and CSV export (retained from original project).
- C HTTP backend and local binary file storage. No database required for the local demo.

## Important security limitations

**LOCAL DEMO ONLY. DO NOT EXPOSE THIS SERVER TO THE PUBLIC INTERNET.** The admin dashboard and administrative API routes are not authenticated, and the C HTTP server is a minimal single-process prototype without TLS, production request handling, or access control. The public booking endpoint uses GET requests. Ticket lookup uses booking ID and phone, which is not strong authentication. Before production, add proper login/roles, secure POST endpoints, HTTPS, CSRF protection, data protection, robust validation, concurrency-safe storage, backups, and a production-grade server. GitHub Pages cannot run the C backend.

## GitHub upload

1. Create a new GitHub repository called `BusNova-C-Reservation-System` (do not initialize with README).
2. In this extracted folder, run in Git Bash or MSYS2 UCRT64:

```bash
git init
git add .
git commit -m "Add BusNova passenger and admin dashboards"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/BusNova-C-Reservation-System.git
git push -u origin main
```

Replace `YOUR_USERNAME` with your actual GitHub username. Authenticate via GitHub's supported sign-in flow. Never put a password or personal access token into source files. If `git` is missing, install Git for Windows from https://git-scm.com/downloads/win.

Alternative: GitHub > New repository > Add file > Upload files, then drag in `server.c`, `index.html`, `admin.html`, `README.md`, and `.gitignore` (not any `.dat` or `.exe`).

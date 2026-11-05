# BusNova public demo deployment

This is a **demonstration system**, not a production ticketing service.

1. Upload the project to a GitHub repository (do not commit `*.dat`, `.env` or credentials).
2. In Render, create a **Web Service** from the repository and select **Docker**. Choose the free plan if available.
3. Add environment variable `ADMIN_PASSWORD` with a strong unique password (at least 16 characters). Never put it in the repository. The `render.yaml` blueprint can generate one automatically if deploying as a Blueprint; retrieve its value securely from Render.
4. Deploy. Open the assigned `https://...onrender.com/` address for passengers and `/admin` for admin.
5. For admin, sign in with username `admin` and the value of `ADMIN_PASSWORD`. Do not share this password with classmates.
6. Create a fictional sample bus from the admin page, then book a fictional passenger from a separate phone.

## Important limitations
- On free Render web services, local `*.dat` files are **ephemeral**. Demo records may disappear on restart or redeploy. Re-create sample data before presenting. No durable database is included.
- The original C HTTP server is single-threaded, uses query-string writes, and has no request rate limiting. Admin routes now require HTTP Basic authentication, but this is **not a security-reviewed production application**. Use fictional data only, never real passenger phone numbers or payments.
- On free services, the instance may sleep when inactive. Visit it before your presentation to warm it up.
- Browser may cache Basic credentials. Close the browser or use a private window to log out.
- If Render does not provide a free service for your account, stop rather than choosing a paid plan.

## Local Linux smoke test
`gcc -O2 server.c -o busnova && ADMIN_PASSWORD='choose-a-test-password' PORT=8080 ./busnova`

Open `http://127.0.0.1:8080/` and `/admin`.

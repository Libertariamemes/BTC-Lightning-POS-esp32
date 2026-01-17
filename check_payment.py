your keyfrom fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import subprocess

app = FastAPI()

# Enable CORS for all origins (you can restrict it later)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Use specific domains in production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ✅ New LNbits HTTPS service address
API_URL = "https://uvlnbits.libertariamemes.com.br/api/v1/payments"

# Request body model
class PaymentData(BaseModel):
    out: bool
    amount: float
    memo: str

@app.post("/send")
def run_curl(data: PaymentData):
    try:
        result = subprocess.run(
            [
                "curl", "-sS",
                "-X", "POST",
                API_URL,
                "-H", "X-Api-Key: make your key at libertariamemes.com.br",
                "-H", "Content-Type: application/json",
                "-d", f'{{"out": {str(data.out).lower()}, "amount": {data.amount}, "memo": "{data.memo}"}}'
            ],
            capture_output=True,
            text=True
        )
        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr
        }
    except Exception as e:
        return {"error": str(e)}
root@vps59583:~/myapi# ^C
root@vps59583:~/myapi# ^C
root@vps59583:~/myapi# ls
check_payment.py  main5.log  main5.py  main6.py  main7.py  main.py  output.log  __pycache__  uvicorn.log  venv
root@vps59583:~/myapi# cat check_payment.py
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import subprocess

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ✅ New HTTPS LNbits URL
API_URL = "https://uvlnbits.libertariamemes.com.br/api/v1/payments"
API_KEY = "make your keys at at https://uvlnbits.libertariamemes.com.br"

class PaymentCheck(BaseModel):
    payment_hash: str

@app.post("/check_payment")
def check_payment(data: PaymentCheck):
    try:
        # Step 1: Silent refresh (like F5)
        subprocess.run(
            [
                "curl", "-sS",
                API_URL,
                "-H", f"X-Api-Key: {API_KEY}",
                "-o", "/dev/null"
            ],
            capture_output=True,
            text=True
        )

        # Step 2: Actual invoice check
        result = subprocess.run(
            [
                "curl", "-sS",
                f"{API_URL}/{data.payment_hash}",
                "-H", f"X-Api-Key: {API_KEY}",
                "-H", "Content-Type: application/json"
            ],
            capture_output=True,
            text=True
        )

        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr
        }

    except Exception as e:
        return {"success": False, "stderr": str(e)}

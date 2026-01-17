from fastapi import FastAPI
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
                "-H", "X-Api-Key: get yourkey at libertariamemes.com.br",
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

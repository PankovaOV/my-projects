FROM python:3.9-slim

WORKDIR /app

RUN pip install --no-cache-dir requests==2.26.0

COPY . .

CMD ["python", "client.py"]
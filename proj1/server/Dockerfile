FROM python:3.9-slim

WORKDIR /app

RUN pip install --no-cache-dir flask==2.3.2 werkzeug==2.3.7

COPY server.py .

EXPOSE 5000
CMD ["python", "server.py"]
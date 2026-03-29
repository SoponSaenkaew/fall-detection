# ใช้ Go build stage
FROM golang:1.26-alpine AS builder

WORKDIR /app

# โหลด dependency ก่อน (cache layer)
COPY go.mod go.sum ./
RUN go mod download

# copy source
COPY . .

# build binary
RUN go build -o server ./cmd/server/main.go

# -------------------------

# runtime stage (เล็กและเร็ว)
FROM alpine:latest

WORKDIR /app

# copy binary จาก builder
COPY --from=builder /app/server .

# port ที่ใช้
EXPOSE 8080

# run
CMD ["./server"]
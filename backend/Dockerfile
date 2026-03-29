# Stage 1: Build stage
FROM golang:1.26-alpine AS builder

WORKDIR /app

# ติดตั้ง build-base สำหรับ cgo (ถ้าจำเป็น)
RUN apk add --no-cache build-base

# โหลด dependency (ใช้ cache layer เพื่อความเร็ว)
COPY go.mod go.sum ./
RUN go mod download

# Copy source code ทั้งหมด
COPY . .

# Build binary
RUN go build -o server ./cmd/server/main.go

# -------------------------
# Stage 2: Runtime stage (ใช้ alpine ตัวเล็กเพื่อความประหยัดพื้นที่)
FROM alpine:latest

# ติดตั้ง ca-certificates สำหรับการส่ง HTTPS (สำคัญมากสำหรับการส่ง LINE/Discord!)
RUN apk --no-cache add ca-certificates

WORKDIR /app

# Copy binary จาก builder stage
COPY --from=builder /app/server .

# ✨ จุดสำคัญ: Copy ไฟล์ .env มาด้วยเพื่อให้โปรแกรมอ่าน Config ได้ค่ะ
COPY .env .

# Port ที่ Backend รันอยู่
EXPOSE 8080

# สั่งรันโปรแกรม
CMD ["./server"]
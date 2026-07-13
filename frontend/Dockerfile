# Stage 1: Build stage
FROM node:20-alpine AS builder

WORKDIR /app

# ติดตั้ง package dependencies
COPY package*.json ./
RUN npm ci

# Copy source code ทั้งหมด
COPY . .

# กำหนด Base API URL สำหรับรันในโหมด Static Export (ใช้พาร์ทสัมพันธ์ /api/v1)
ARG NEXT_PUBLIC_API_URL=/api/v1
ENV NEXT_PUBLIC_API_URL=$NEXT_PUBLIC_API_URL

# Build และ Export หน้าเว็บเป็น HTML/CSS/JS
RUN npm run build

# Stage 2: Serve stage with Nginx (เพื่อความปลอดภัยและใช้แรมเพียง ~2-3MB)
FROM nginx:alpine

# Copy ไฟล์ HTML ที่คอมไพล์สำเร็จแล้วไปยังโฟลเดอร์รันหน้าเว็บของ Nginx
COPY --from=builder /app/out /usr/share/nginx/html

# ตั้งค่า Nginx ให้ทำงานเป็น Single Page Application (SPA) Routing สำหรับ Next.js Static Export
RUN echo 'server { \
    listen 80; \
    location / { \
        root /usr/share/nginx/html; \
        try_files $uri $uri.html $uri/ /index.html; \
    } \
}' > /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]

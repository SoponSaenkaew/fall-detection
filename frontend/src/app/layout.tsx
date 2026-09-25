import type { Metadata } from "next";
import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "SafeBathroom IoT - ระบบตรวจจับการล้มและดูแลความปลอดภัยในห้องน้ำ",
  description: "ระบบตรวจจับการล้มและดูแลความปลอดภัยในห้องน้ำอัจฉริยะด้วยเทคโนโลยีเรดาร์คลื่นมิลลิเมตร (mmWave)",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html
      lang="en"
      className={`${geistSans.variable} ${geistMono.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col">{children}</body>
    </html>
  );
}

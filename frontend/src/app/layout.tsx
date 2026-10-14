import type { Metadata } from "next";
import { Prompt } from "next/font/google";
import "./globals.css";

const promptFont = Prompt({
  variable: "--font-prompt",
  subsets: ["latin", "thai"],
  weight: ["300", "400", "500", "600", "700"],
  display: "swap",
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
      lang="th"
      className={`${promptFont.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col font-sans bg-slate-50 text-slate-900">{children}</body>
    </html>
  );
}

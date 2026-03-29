// frontend/src/app/layout.tsx
import "./globals.css"; // ดึง Style ของ Tailwind มาใช้ในทุกหน้าค่ะ

export const metadata = {
  title: "Fall Detection System",
  description: "ระบบตรวจจับการล้ม",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="th">
      <body className="antialiased">
        {/* children คือส่วนที่จะเอา page.tsx ของหน้าต่างๆ มาใส่ตรงนี้ค่ะ */}
        <main>{children}</main>
      </body>
    </html>
  );
}
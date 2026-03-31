"use client";
import { useEffect, useState } from 'react';
import { groupService, deviceService, notificationService } from '@/services/api';
import DeviceCard from '@/components/Dashboard/DeviceCard';
import { Layout, Plus, Trash2, X, SmartphoneNfc, BellRing } from 'lucide-react';
import Header from '@/components/Dashboard/Header';

export default function Dashboard() {
  const [groups, setGroups] = useState<any[]>([]);
  const [showAddModal, setShowAddModal] = useState(false);
  const [newGroupName, setNewGroupName] = useState("");
  const [showAddDevModal, setShowAddDevModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [newDev, setNewDev] = useState({ device_id: "", name: "" });
  const [showNotifModal, setShowNotifModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [notifForm, setNotifForm] = useState({ type: 'line', line_token: '', line_group_id: '', target_url: '' });


  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  const fetchData = async () => {
    try {
      const res = await groupService.getAll();
      setGroups(res.data);
    } catch (err) { console.error("ดึงข้อมูลไม่สำเร็จ"); }
  };
  // --- Handler สำหรับการลบกลุ่ม (Group) ---
  const handleDeleteGroup = async (id: number) => {
    if (!confirm("เซนเซย์แน่ใจนะค๊ะว่าจะลบกลุ่มนี้? ข้อมูลอุปกรณ์ข้างในจะหายไปด้วยนะ!")) return;
    try {
      // เรียกใช้ service ที่แยกไว้
      await groupService.delete(id); 
      alert("ลบสถานที่เรียบร้อยแล้วค่ะ! ✨");
      fetchData(); // โหลดข้อมูลใหม่เพื่ออัปเดตหน้าจอ
    } catch (err: any) {
      alert(err.response?.data?.error || "ลบไม่สำเร็จค่ะ");
    }
  };

  // --- Handler สำหรับการลบอุปกรณ์ (Device) ---
  const handleDeleteDevice = async (deviceId: string) => {
    if (!confirm(`จะลบอุปกรณ์รหัส ${deviceId} ใช่ไหมคะเซนเซย์?`)) return;
    try {
      // เรียกใช้ service ที่แยกไว้
      await deviceService.delete(deviceId);
      alert("ลบอุปกรณ์เรียบร้อยแล้วค่ะ! ✨");
      fetchData(); // อัปเดตรายการอุปกรณ์ในกลุ่ม
    } catch (err: any) {
      alert(err.response?.data?.error || "ลบอุปกรณ์ไม่สำเร็จค่ะ");
    }
  };

  // ... รวม Handler ต่างๆ ไว้ที่นี่ (handleAddGroup, handleDeleteGroup, handleAddDevice, handleSaveNotif) ...
  // ทุกฟังก์ชันจะเปลี่ยนไปเรียกใช้ service แทน เช่น groupService.create(newGroupName)

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 5000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      {/* Header, Modals ต่างๆ ยังคงอยู่ตรงนี้ หรือจะแยกออกไปอีกก็ได้ค่ะ */}
      <Header onAddGroup={() => setShowAddModal(true)} />
      
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-6 rounded-2xl shadow-sm border border-gray-200 relative group">
            {/* Group Header & Actions */}
            <div className="absolute top-4 right-4 flex gap-2 opacity-0 group-hover:opacity-100 transition">
              <button onClick={() => setShowNotifModal({show: true, groupId: group.ID})} className="text-gray-400 hover:text-yellow-500"><BellRing size={18} /></button>
              <button onClick={() => handleDeleteGroup(group.ID)} className="text-gray-400 hover:text-red-500"><Trash2 size={18} /></button>
            </div>
            
            <h2 className="text-xl font-bold flex items-center mb-4 text-blue-600 border-b pb-2"><Layout className="mr-2" />{group.name}</h2>
            
            <div className="space-y-4">
              {group.devices?.map((dev: any) => (
                <DeviceCard key={dev.ID} dev={dev} formatTime={formatTime} onDelete={handleDeleteDevice} />
              ))}
              
              <button onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} className="w-full py-2 border-2 border-dashed border-blue-100 rounded-xl text-blue-400 text-xs hover:bg-blue-50 transition flex items-center justify-center">
                <Plus size={14} className="mr-1" /> เพิ่มอุปกรณ์
              </button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
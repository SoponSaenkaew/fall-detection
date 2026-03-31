"use client";
import { useEffect, useState } from 'react';
import axios from 'axios';
import { Layout, Smartphone, Plus, Trash2, X, Clock, UserCheck, SmartphoneNfc, BellRing } from 'lucide-react';

export default function Dashboard() {
  const [groups, setGroups] = useState<any[]>([]);
  const [showAddModal, setShowAddModal] = useState(false);
  const [newGroupName, setNewGroupName] = useState("");

  // สถานะสำหรับ Modal เพิ่มอุปกรณ์
  const [showAddDevModal, setShowAddDevModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [newDev, setNewDev] = useState({ device_id: "", name: "" });

  // ✨ สถานะสำหรับ Modal ตั้งค่าการแจ้งเตือน
  const [showNotifModal, setShowNotifModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [notifForm, setNotifForm] = useState({
    type: 'line',
    line_token: '',
    line_group_id: '',
    target_url: '',
  });

  const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;
  const apiConfig = { headers: { Authorization: `Bearer ${token}` } };

  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  const fetchData = async () => {
    try {
      const res = await axios.get(`${process.env.NEXT_PUBLIC_API_URL}/iot/groups`, apiConfig);
      setGroups(res.data);
    } catch (err) {
      console.error("ดึงข้อมูลไม่สำเร็จค่ะเซนเซย์");
    }
  };

  const handleAddGroup = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await axios.post(`${process.env.NEXT_PUBLIC_API_URL}/iot/groups`, { name: newGroupName }, apiConfig);
      setNewGroupName("");
      setShowAddModal(false);
      fetchData();
    } catch (err) { alert("สร้างกลุ่มไม่สำเร็จค่ะ"); }
  };

  const handleDeleteGroup = async (id: number) => {
    if (!confirm("เซนเซย์แน่ใจนะค๊ะว่าจะลบกลุ่มนี้? ข้อมูลอุปกรณ์ข้างในจะหายไปด้วยนะ!")) return;
    try {
      await axios.delete(`${process.env.NEXT_PUBLIC_API_URL}/iot/groups/${id}`, apiConfig);
      fetchData();
    } catch (err) { alert("ลบไม่สำเร็จค่ะ"); }
  };

  const handleAddDevice = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await axios.post(`${process.env.NEXT_PUBLIC_API_URL}/iot/devices`, { ...newDev, group_id: showAddDevModal.groupId }, apiConfig);
      setNewDev({ device_id: "", name: "" });
      setShowAddDevModal({ show: false, groupId: null });
      fetchData();
    } catch (err) { alert("ลงทะเบียนอุปกรณ์ไม่สำเร็จค่ะ"); }
  };

  const handleDeleteDevice = async (deviceId: string) => {
    if (!confirm(`จะลบอุปกรณ์รหัส ${deviceId} ใช่ไหมคะเซนเซย์?`)) return;
    try {
      await axios.delete(`${process.env.NEXT_PUBLIC_API_URL}/iot/devices/${deviceId}`, apiConfig);
      fetchData();
    } catch (err) { alert("ลบอุปกรณ์ไม่สำเร็จค่ะ"); }
  };

  // ✨ ฟังก์ชันบันทึกการตั้งค่าการแจ้งเตือน
  const handleSaveNotif = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await axios.post(`${process.env.NEXT_PUBLIC_API_URL}/iot/notifications`, 
        { ...notifForm, group_id: showNotifModal.groupId },
        apiConfig
      );
      alert("บันทึกการตั้งค่าสำเร็จแล้วค่ะ! ✨");
      setShowNotifModal({ show: false, groupId: null });
    } catch (err) { alert("บันทึกไม่สำเร็จค่ะเซนเซย์"); }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 5000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      <header className="flex justify-between items-center mb-8 border-b pb-4">
        <div>
          <h1 className="text-3xl font-bold text-blue-800">IoT Dashboard 🖥️</h1>
          <p className="text-gray-500 text-sm">อัปเดตล่าสุด: {new Date().toLocaleTimeString('th-TH')}</p>
        </div>
        <button onClick={() => setShowAddModal(true)} className="flex items-center bg-blue-600 text-white px-4 py-2 rounded-lg hover:bg-blue-700 transition shadow-lg">
          <Plus size={20} className="mr-2" /> เพิ่มกลุ่มใหม่
        </button>
      </header>

      {/* Modal เพิ่มกลุ่ม */}
      {showAddModal && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50">
          <form onSubmit={handleAddGroup} className="bg-white p-6 rounded-xl w-96 shadow-2xl border-t-4 border-blue-500">
            <div className="flex justify-between mb-4 italic text-blue-600">
              <h3 className="text-xl font-bold">สร้างสถานที่ใหม่นะเซนเซย์! 🏠</h3>
              <button type="button" onClick={() => setShowAddModal(false)}><X /></button>
            </div>
            <input autoFocus className="w-full border p-2 rounded-lg mb-4" placeholder="เช่น ห้องน้ำชั้น 2" value={newGroupName} onChange={(e) => setNewGroupName(e.target.value)} required />
            <button type="submit" className="w-full bg-blue-600 text-white py-2 rounded-lg font-bold">ยืนยัน ✨</button>
          </form>
        </div>
      )}

      {/* ✨ Modal ตั้งค่าการแจ้งเตือน */}
      {showNotifModal.show && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
          <form onSubmit={handleSaveNotif} className="bg-white p-6 rounded-2xl w-full max-w-md shadow-2xl border-t-4 border-yellow-400">
            <div className="flex justify-between mb-4 text-yellow-600">
              <h3 className="text-xl font-bold flex items-center"><BellRing className="mr-2" /> ตั้งค่าการแจ้งเตือน</h3>
              <button type="button" onClick={() => setShowNotifModal({show: false, groupId: null})}><X /></button>
            </div>
            <div className="space-y-4">
              <label className="block text-sm font-medium text-gray-700 text-left">ประเภทการแจ้งเตือน</label>
              <select className="w-full border p-2 rounded-lg" value={notifForm.type} onChange={(e) => setNotifForm({...notifForm, type: e.target.value})}>
                <option value="line">LINE Notify / Messaging API</option>
                <option value="webhook">Webhook URL (Discord)</option>
              </select>
              {notifForm.type === 'line' ? (
                <>
                  <input className="w-full border p-2 rounded-lg" placeholder="LINE Token" onChange={(e) => setNotifForm({...notifForm, line_token: e.target.value})} required />
                  <input className="w-full border p-2 rounded-lg" placeholder="LINE User/Group ID" onChange={(e) => setNotifForm({...notifForm, line_group_id: e.target.value})} required />
                </>
              ) : (
                <input className="w-full border p-2 rounded-lg" placeholder="Webhook URL (เช่น https://discord.com/api/webhooks/...)" onChange={(e) => setNotifForm({...notifForm, target_url: e.target.value})} required />
              )}
              <button type="submit" className="w-full bg-yellow-500 text-white py-2 rounded-lg font-bold hover:bg-yellow-600">บันทึกการตั้งค่า ✨</button>
            </div>
          </form>
        </div>
      )}

      {/* Modal เพิ่มอุปกรณ์ */}
      {showAddDevModal.show && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
          <form onSubmit={handleAddDevice} className="bg-white p-6 rounded-2xl w-full max-w-sm shadow-2xl border-t-4 border-green-500">
            <div className="flex justify-between mb-4 text-green-700">
              <h3 className="text-xl font-bold flex items-center"><SmartphoneNfc className="mr-2" /> เพิ่มเซนเซอร์ใหม่</h3>
              <button type="button" onClick={() => setShowAddDevModal({show: false, groupId: null})}><X /></button>
            </div>
            <div className="space-y-4">
              <input className="w-full border p-2 rounded-lg" placeholder="Hardware ID" value={newDev.device_id} onChange={(e) => setNewDev({...newDev, device_id: e.target.value})} required />
              <input className="w-full border p-2 rounded-lg" placeholder="ชื่อเรียก" value={newDev.name} onChange={(e) => setNewDev({...newDev, name: e.target.value})} required />
              <button type="submit" className="w-full bg-green-600 text-white py-2 rounded-lg font-bold">ลงทะเบียน ✨</button>
            </div>
          </form>
        </div>
      )}

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-6 rounded-2xl shadow-sm border border-gray-200 relative group">
            <div className="absolute top-4 right-4 flex gap-2 opacity-0 group-hover:opacity-100 transition">
              {/* ✨ ปุ่มตั้งค่าการแจ้งเตือน */}
              <button onClick={() => setShowNotifModal({show: true, groupId: group.ID})} className="text-gray-400 hover:text-yellow-500" title="ตั้งค่าการแจ้งเตือน">
                <BellRing size={18} />
              </button>
              <button onClick={() => handleDeleteGroup(group.ID)} className="text-gray-400 hover:text-red-500" title="ลบกลุ่มนี้">
                <Trash2 size={18} />
              </button>
            </div>
            
            <div className="flex items-center mb-4 text-blue-600 border-b pb-2 text-left">
              <Layout className="mr-2" />
              <h2 className="text-xl font-bold">{group.name}</h2>
            </div>
            
            <div className="space-y-4">
              {group.devices?.map((dev: any) => (
                <div key={dev.ID} className="p-4 bg-slate-50 rounded-xl border border-gray-100 relative group/dev">
                  <button onClick={() => handleDeleteDevice(dev.device_id)} className="absolute top-2 right-2 text-gray-300 hover:text-red-500 transition opacity-0 group-hover/dev:opacity-100">
                    <Trash2 size={14} />
                  </button>
                  <div className="flex justify-between items-start mb-2">
                    <div className="flex items-center font-medium text-gray-700 text-sm">
                      <Smartphone size={16} className="mr-2 text-blue-400" />
                      {dev.name}
                    </div>
                    <span className={`text-[10px] px-2 py-0.5 rounded-full font-bold uppercase ${dev.status === 'online' ? 'bg-green-100 text-green-600' : 'bg-red-100 text-red-600'}`}>
                      {dev.status}
                    </span>
                  </div>
                  <div className="flex flex-col gap-2 mt-3 p-3 bg-white rounded-lg shadow-sm border border-blue-100">
                    <div className="flex items-center justify-between">
                      <div className="flex items-center text-sm text-gray-600"><UserCheck size={16} className="mr-2 text-blue-400" /><span>สถานะ:</span></div>
                      <span className={`text-sm font-bold ${dev.latest_event === 'fall' ? 'text-red-500 animate-bounce' : 'text-blue-600'}`}>
                        {dev.latest_event === 'enter' ? 'กำลังใช้งาน 🚪' : dev.latest_event === 'exit' ? 'ว่าง 🟢' : dev.latest_event === 'fall' ? 'คนล้ม!!! ⚠️' : 'ไม่มีข้อมูล'}
                      </span>
                    </div>
                    <div className="flex items-center justify-between text-[11px] text-gray-400">
                      <div className="flex items-center"><Clock size={12} className="mr-1" />อัปเดตเมื่อ:</div>
                      <span>{formatTime(dev.UpdatedAt)}</span>
                    </div>
                  </div>
                </div>
              ))}
              <button onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} className="w-full py-2 border-2 border-dashed border-blue-100 rounded-xl text-blue-400 text-xs hover:bg-blue-50 transition flex items-center justify-center font-medium">
                <Plus size={14} className="mr-1" /> เพิ่มอุปกรณ์ใน {group.name}
              </button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
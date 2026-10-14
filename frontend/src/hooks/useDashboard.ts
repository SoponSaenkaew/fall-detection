import { useState, useEffect } from 'react';
import { groupService, deviceService, notificationService } from '@/services/api';

export const useDashboard = () => {
  const [groups, setGroups] = useState<any[]>([]);
  
  // States สำหรับควบคุม Modal
  const [showAddModal, setShowAddModal] = useState(false);
  const [newGroupName, setNewGroupName] = useState("");
  const [showAddDevModal, setShowAddDevModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [newDev, setNewDev] = useState({ device_id: "", name: "" });
  const [showNotifModal, setShowNotifModal] = useState<{show: boolean, groupId: number | null}>({show: false, groupId: null});
  const [notifForm, setNotifForm] = useState({ type: 'line', line_token: '', line_group_id: '', target_url: '' });
  
  const [showEditGroup, setShowEditGroup] = useState<{show: boolean, id: number | null}>({show: false, id: null});
  const [editGroupName, setEditGroupName] = useState("");
  const [showEditDev, setShowEditDev] = useState<{show: boolean, originalId: string | null}>({show: false, originalId: null});
  const [editDevData, setEditDevData] = useState({ device_id: "", name: "" });

  const [showSettings, setShowSettings] = useState({ show: false, deviceId: "" });
  const [currentSettings, setCurrentSettings] = useState({
    device_id: "",
    stay_timeout_minutes: 30.0,
    fall_threshold: 0.5,
    mount_height: 2.0,
    room_width: 4.0,
    room_length: 4.0
  });

  const fetchData = async () => {
    try {
      const res = await groupService.getAll();
      setGroups(res.data);
    } catch (err) { console.error("ดึงข้อมูลไม่สำเร็จ"); }
  };



  // ✨ ฟังก์ชันบันทึกการแก้กลุ่ม
  const handleUpdateGroup = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!showEditGroup.id) return;
    try {
      await groupService.update(showEditGroup.id, editGroupName);
      setShowEditGroup({ show: false, id: null });
      fetchData();
    } catch (err) { alert("แก้ไขไม่สำเร็จค่ะ"); }
  };

  // ✨ ฟังก์ชันบันทึกการแก้ตัวเครื่อง
  const handleUpdateDevice = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!showEditDev.originalId) return;

    const trimmedId = editDevData.device_id.trim();
    if (!trimmedId) {
      alert("กรุณาระบุรหัสอุปกรณ์");
      return;
    }

    // ตรวจสอบเบื้องต้นว่า Device ID ใหม่ซ้ำกับอุปกรณ์อื่นที่อยู่ในระบบหรือไม่
    if (trimmedId.toLowerCase() !== showEditDev.originalId.toLowerCase()) {
      const isDuplicate = groups.some((g: any) =>
        g.devices?.some((d: any) => 
          d.device_id?.toLowerCase() === trimmedId.toLowerCase() && 
          d.device_id?.toLowerCase() !== showEditDev.originalId?.toLowerCase()
        )
      );
      if (isDuplicate) {
        alert(`รหัสอุปกรณ์ '${trimmedId}' มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น`);
        return;
      }
    }

    try {
      await deviceService.update(showEditDev.originalId, { ...editDevData, device_id: trimmedId });
      setShowEditDev({ show: false, originalId: null });
      fetchData();
    } catch (err: any) { 
      alert(err.response?.data?.error || "อัปเดตข้อมูลเซนเซอร์ไม่สำเร็จค่ะ"); 
    }
  };

  const handleAddGroup = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await groupService.create(newGroupName);
      setNewGroupName("");
      setShowAddModal(false);
      fetchData();
    } catch (err) { alert("สร้างกลุ่มไม่สำเร็จ"); }
  };

  const handleDeleteGroup = async (id: number) => {
    if (!confirm("คุณแน่ใจหรือไม่ว่าต้องการลบกลุ่มนี้?")) return;
    try {
      await groupService.delete(id);
      fetchData();
    } catch (err) { alert("ลบไม่สำเร็จ"); }
  };

  const handleAddDevice = async (e: React.FormEvent) => {
    e.preventDefault();
    const trimmedId = newDev.device_id.trim();
    if (!trimmedId) {
      alert("กรุณาระบุรหัสอุปกรณ์");
      return;
    }

    // ตรวจสอบเบื้องต้นว่า Device ID ซ้ำกับอุปกรณ์ที่มีอยู่แล้วในระบบหรือไม่
    const isDuplicate = groups.some((g: any) =>
      g.devices?.some((d: any) => d.device_id?.toLowerCase() === trimmedId.toLowerCase())
    );
    if (isDuplicate) {
      alert(`รหัสอุปกรณ์ '${trimmedId}' มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น`);
      return;
    }

    try {
      await deviceService.create({ ...newDev, device_id: trimmedId, group_id: showAddDevModal.groupId });
      setNewDev({ device_id: "", name: "" });
      setShowAddDevModal({ show: false, groupId: null });
      fetchData();
    } catch (err: any) { 
      alert(err.response?.data?.error || "ลงทะเบียนไม่สำเร็จ"); 
    }
  };

  const handleDeleteDevice = async (deviceId: string) => {
    if (!confirm(`จะลบอุปกรณ์รหัส ${deviceId} ใช่ไหมคะ?`)) return;
    try {
      await deviceService.delete(deviceId);
      fetchData();
    } catch (err) { alert("ลบอุปกรณ์ไม่สำเร็จค่ะ"); }
  };

  const handleSaveNotif = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await notificationService.save({ ...notifForm, type: 'line', group_id: showNotifModal.groupId });
      alert("บันทึกการตั้งค่าแจ้งเตือน LINE เรียบร้อยแล้วค่ะ! ✨");
      setShowNotifModal({ show: false, groupId: null });
    } catch (err) { alert("บันทึกไม่สำเร็จค่ะ"); }
  };


  // ✨ ฟังก์ชันเปิด Modal และดึงค่าจาก API
  const handleOpenSettings = async (deviceId: string) => {
    try {
      const res = await deviceService.getSettings(deviceId);
      setCurrentSettings({
        ...res.data,
        mount_height: res.data?.mount_height || 2.2,
        room_width: res.data?.room_width || 2.0,
        room_length: res.data?.room_length || 2.0,
        stay_timeout_minutes: res.data?.stay_timeout_minutes || 30.0,
        fall_threshold: res.data?.fall_threshold || 0.5,
      });
      setShowSettings({ show: true, deviceId });
    } catch (err) {
      // ถ้ายังไม่มีค่าใน DB ให้ใช้ค่าแนะนำของระบบ (สูง 2.2ม., กว้าง 2.0ม., ยาว 2.0ม.)
      setCurrentSettings({
        device_id: deviceId,
        stay_timeout_minutes: 30.0,
        fall_threshold: 0.5,
        mount_height: 2.2,
        room_width: 2.0,
        room_length: 2.0
      });
      setShowSettings({ show: true, deviceId });
    }
  };

  // ฟังก์ชันบันทึกค่าพารามิเตอร์
  const handleSaveSettings = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await deviceService.updateSettings(currentSettings);
      alert("บันทึกค่าพารามิเตอร์ลงตัวเครื่องสำเร็จเรียบร้อย");
      setShowSettings({ show: false, deviceId: "" });
      fetchData(); // รีเฟรชข้อมูลเพื่อให้อัปเดตค่าพารามิเตอร์ล่าสุดบนการ์ด
    } catch (err: any) {
      alert(err.response?.data?.error || "บันทึกไม่สำเร็จ โปรดตรวจสอบความถูกต้องของข้อมูล");
    }
  };

  useEffect(() => {
    fetchData();
    // const interval = setInterval(fetchData, 5000);
    // return () => clearInterval(interval);
  }, []);

  // คืนค่าทุกอย่างเพื่อให้หน้า Page นำไปใช้
  return {
    // ... คืนค่าเดิมทั้งหมด ...
    groups, fetchData,
    showAddModal, setShowAddModal,
    newGroupName, setNewGroupName,
    showAddDevModal, setShowAddDevModal,
    newDev, setNewDev,
    showNotifModal, setShowNotifModal,
    notifForm, setNotifForm,
    
    // ✨ คืนค่าใหม่ที่เพิ่มเข้ามา
    showEditGroup, setShowEditGroup,
    editGroupName, setEditGroupName,
    showEditDev, setShowEditDev,
    editDevData, setEditDevData,
    handleUpdateGroup,
    handleUpdateDevice,

    showSettings, setShowSettings,
    currentSettings, setCurrentSettings,
    handleOpenSettings, handleSaveSettings,

    // ... Handler เดิม ...
    handleAddGroup, handleDeleteGroup,
    handleAddDevice, handleDeleteDevice,
    handleSaveNotif
  };
};
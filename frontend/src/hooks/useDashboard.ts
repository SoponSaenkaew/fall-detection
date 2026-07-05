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
    try {
      await deviceService.update(showEditDev.originalId, editDevData);
      setShowEditDev({ show: false, originalId: null });
      fetchData();
    } catch (err) { alert("อัปเดตข้อมูลเซนเซอร์ไม่สำเร็จค่ะ"); }
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
    try {
      await deviceService.create({ ...newDev, group_id: showAddDevModal.groupId });
      setNewDev({ device_id: "", name: "" });
      setShowAddDevModal({ show: false, groupId: null });
      fetchData();
    } catch (err) { alert("ลงทะเบียนไม่สำเร็จ"); }
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
      await notificationService.save({ ...notifForm, group_id: showNotifModal.groupId });
      alert("บันทึกการตั้งค่าสำเร็จแล้วค่ะ! ✨");
      setShowNotifModal({ show: false, groupId: null });
    } catch (err) { alert("บันทึกไม่สำเร็จค่ะ"); }
  };


  // ✨ ฟังก์ชันเปิด Modal และดึงค่าจาก API
  const handleOpenSettings = async (deviceId: string) => {
    try {
      const res = await deviceService.getSettings(deviceId);
      setCurrentSettings(res.data);
      setShowSettings({ show: true, deviceId });
    } catch (err) {
      // ถ้ายังไม่มีค่าใน DB ให้ใช้ค่าเริ่มต้นและผูกกับ DeviceID นั้นๆ ค่ะ
      setCurrentSettings({
        device_id: deviceId,
        fall_threshold: 0.5,
        mount_height: 2.0,
        room_width: 4.0,
        room_length: 4.0
      });
      setShowSettings({ show: true, deviceId });
    }
  };

  // ✨ ฟังก์ชันบันทึกค่าพารามิเตอร์
  const handleSaveSettings = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await deviceService.updateSettings(currentSettings);
      alert("บันทึกค่าพารามิเตอร์สำเร็จ");
      setShowSettings({ show: false, deviceId: "" });
    } catch (err) { alert("บันทึกไม่สำเร็จ"); }
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
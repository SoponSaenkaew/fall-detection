export interface Device {
  ID: number;
  device_id: string;
  name: string;
  status: string;
}

export interface Group {
  ID: number;
  name: string;
  devices: Device[];
}
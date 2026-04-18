import asyncio
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
COMMAND_CHAR_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

async def test():
    print("🔍 Scanning...")
    devices = await BleakScanner.discover()
    
    for device in devices:
        print(f"  {device.name}: {device.address}")
    
    # Find robot
    robot = None
    for device in devices:
        if "Arduino" in (device.name or ""):
            robot = device
            break
    
    if not robot:
        print("❌ Robot not found")
        return
    
    print(f"\n✅ Found: {robot.name}")
    
    # Connect
    async with BleakClient(robot.address) as client:
        print("🔵 Connected!")
        
        # Send test command
        test_cmd = "1u"
        print(f"\n📤 Sending test command: {test_cmd}")
        await client.write_gatt_char(COMMAND_CHAR_UUID, test_cmd.encode())
        print("✅ Command sent!")
        
        await asyncio.sleep(2)
    
    print("✅ Test complete!")

asyncio.run(test())
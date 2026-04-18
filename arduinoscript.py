import asyncio
import sys
from bleak import BleakClient, BleakScanner
from pynput import keyboard

# BLE UUIDs (must match Arduino)
SERVICE_UUID      = "19B10000-E8F2-537E-4F6C-D104768A1214"
COMMAND_CHAR_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

client        = None
command_queue = None


async def connect_to_robot():
    global client

    print("🔍 Scanning for RobotArm...")
    devices = await BleakScanner.discover()

    robot_address = None
    for device in devices:
        print(f"  Found: {device.name}")
        if device.name == "RobotArm":
            robot_address = device.address
            print(f"\n✅ Found RobotArm at: {robot_address}")
            break

    if not robot_address:
        print("❌ RobotArm not found!")
        return None

    try:
        client = BleakClient(robot_address)
        await client.connect()
        print("🔵 Connected to RobotArm!")
        return client
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        return None


async def send_command(command):
    global client

    if not client or not client.is_connected:
        print("❌ Not connected!")
        return False

    try:
        print(f"📤 Sending: {command}")
        await client.write_gatt_char(COMMAND_CHAR_UUID, command.encode())
        await asyncio.sleep(0.05)
        return True
    except Exception as e:
        print(f"❌ Error: {e}")
        return False


async def command_processor():
    while True:
        try:
            command = await asyncio.wait_for(command_queue.get(), timeout=0.1)
            await send_command(command)
        except asyncio.TimeoutError:
            await asyncio.sleep(0.01)
        except Exception as e:
            print(f"Error: {e}")
            await asyncio.sleep(0.1)


def on_press(key):
    global command_queue

    try:
        if hasattr(key, 'char') and key.char:

            # Motor 1 — Base (spins freely)
            if key.char == 'a':
                print("⬅️  Base Left")
                command_queue.put_nowait('1u')
            elif key.char == 'd':
                print("➡️  Base Right")
                command_queue.put_nowait('1d')

            # Motor 2 — Shoulder
            elif key.char == 'w':
                print("⬆️  Shoulder Up")
                command_queue.put_nowait('2u')
            elif key.char == 's':
                print("⬇️  Shoulder Down")
                command_queue.put_nowait('2d')

        # Motor 3 — Elbow (arrow keys)
        elif key == keyboard.Key.up:
            print("⬆️  Elbow Up")
            command_queue.put_nowait('3u')

        elif key == keyboard.Key.down:
            print("⬇️  Elbow Down")
            command_queue.put_nowait('3d')

        elif key == keyboard.Key.esc:
            print("\n🛑 Exiting...")
            return False

    except Exception:
        pass


async def main():
    global client, command_queue

    command_queue = asyncio.Queue()

    client = await connect_to_robot()

    if not client:
        return

    print("\n" + "="*50)
    print("🦾 ROBOT ARM CONTROL")
    print("="*50)
    print("\n🔄 BASE (Motor 1):")
    print("  A = Left           D = Right")
    print("\n💪 SHOULDER (Motor 2):")
    print("  W = Up             S = Down")
    print("\n🦾 ELBOW (Motor 3):")
    print("  ↑ = Up             ↓ = Down")
    print("\n🎮 OTHER:")
    print("  ESC = Exit")
    print("="*50 + "\n")

    processor_task = asyncio.create_task(command_processor())

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    print("🎮 Ready! Press keys...\n")

    try:
        while listener.is_alive():
            await asyncio.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        listener.stop()
        processor_task.cancel()
        if client:
            await client.disconnect()
        print("\n✅ Disconnected")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n🛑 Interrupted")
        sys.exit(0)
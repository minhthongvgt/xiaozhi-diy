import socket
import wave
import argparse


'''
  Create a UDP socket and bind it to the server's IP:8000.
  Listen for incoming messages and print them to the console.
  Save the audio to a WAV file.
'''
def main(samplerate, channels):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind(('0.0.0.0', 8000))

    filename = f"{samplerate}_{channels}.wav"
    wav_file = wave.open(filename, "wb")
    wav_file.setnchannels(channels)
    wav_file.setsampwidth(2)
    wav_file.setframerate(samplerate)

    print(f"Start saving audio from 0.0.0.0:8000 to {filename}...")

    try:
        while True:
            message, address = server_socket.recvfrom(8000)
            
            wav_file.writeframes(message)

            print(f"Received {len(message)} bytes from {address}")
    
    except KeyboardInterrupt:
        print("\nStopping recording...")
    
    finally:
        wav_file.close()
        server_socket.close()
        print(f"WAV file '{filename}' saved successfully")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='UDP音频数据接收器，保存为WAV文件')
    parser.add_argument('--samplerate', '-s', type=int, default=16000, 
                        help='采样率 (默认: 16000)')
    parser.add_argument('--channels', '-c', type=int, default=2, 
                        help='声道数 (默认: 2)')
    
    args = parser.parse_args()
    main(args.samplerate, args.channels)

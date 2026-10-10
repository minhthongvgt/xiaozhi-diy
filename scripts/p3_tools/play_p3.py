import opuslib
import struct
import numpy as np
import sounddevice as sd
import argparse

def play_p3_file(input_file):
    """
    播放p3格式的音频文件
    p3格式: [1字节类型, 1字节保留, 2字节长度, Opus数据]
    """
    sample_rate = 16000
    channels = 1
    decoder = opuslib.Decoder(sample_rate, channels)
    
    frame_size = int(sample_rate * 60 / 1000)
    
    stream = sd.OutputStream(
        samplerate=sample_rate,
        channels=channels,
        dtype='int16'
    )
    stream.start()
    
    try:
        with open(input_file, 'rb') as f:
            print(f"正在播放: {input_file}")
            
            while True:
                header = f.read(4)
                if not header or len(header) < 4:
                    break
                
                packet_type, reserved, data_len = struct.unpack('>BBH', header)
                
                opus_data = f.read(data_len)
                if not opus_data or len(opus_data) < data_len:
                    break
                
                pcm_data = decoder.decode(opus_data, frame_size)
                
                audio_array = np.frombuffer(pcm_data, dtype=np.int16)
                
                stream.write(audio_array)
                
    except KeyboardInterrupt:
        print("\n播放已停止")
    finally:
        stream.stop()
        stream.close()
        print("播放完成")

def main():
    parser = argparse.ArgumentParser(description='播放p3格式的音频文件')
    parser.add_argument('input_file', help='输入的p3文件路径')
    args = parser.parse_args()
    
    play_p3_file(args.input_file)

if __name__ == "__main__":
    main() 

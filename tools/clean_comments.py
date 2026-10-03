import os
import sys
import glob

def remove_c_comments(text):
    out = []
    state = 'NORMAL'
    i = 0
    n = len(text)
    while i < n:
        if state == 'NORMAL':
            if text[i:i+2] == '//':
                state = 'LINE_COMMENT'
                i += 2
            elif text[i:i+2] == '/*':
                state = 'BLOCK_COMMENT'
                i += 2
            elif text[i] == '"':
                state = 'STRING'
                out.append(text[i])
                i += 1
            elif text[i] == "'":
                state = 'CHAR'
                out.append(text[i])
                i += 1
            else:
                out.append(text[i])
                i += 1
        elif state == 'STRING':
            if text[i] == '\\':
                out.append(text[i:i+2])
                i += 2
            elif text[i] == '"':
                state = 'NORMAL'
                out.append(text[i])
                i += 1
            else:
                out.append(text[i])
                i += 1
        elif state == 'CHAR':
            if text[i] == '\\':
                out.append(text[i:i+2])
                i += 2
            elif text[i] == "'":
                state = 'NORMAL'
                out.append(text[i])
                i += 1
            else:
                out.append(text[i])
                i += 1
        elif state == 'LINE_COMMENT':
            if text[i] == '\n':
                state = 'NORMAL'
                out.append('\n')
                i += 1
            else:
                i += 1
        elif state == 'BLOCK_COMMENT':
            if text[i:i+2] == '*/':
                state = 'NORMAL'
                i += 2
            else:
                if text[i] == '\n':
                    out.append('\n')
                i += 1
    
    result = "".join(out)
    # Xóa các dòng trống do xóa comment để lại
    final_lines = []
    for line in result.split('\n'):
        if line.strip() or final_lines and not final_lines[-1].strip():
            # Chỉ giữ lại dòng nếu nó không trống, hoặc cho phép tối đa 1 dòng trống liên tiếp
            pass
        if line.strip() == "":
            continue
        final_lines.append(line)
        
    return '\n'.join(final_lines) + '\n'

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        cleaned = remove_c_comments(content)
        
        if content != cleaned:
            with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
                f.write(cleaned)
            return True
    except Exception as e:
        print(f"Lỗi khi xử lý {filepath}: {e}")
    return False

def main(root_dir):
    extensions = {'.c', '.h', '.cpp', '.hpp', '.js'}
    count = 0
    for root, dirs, files in os.walk(root_dir):
        # Bỏ qua các thư mục không cần thiết
        if any(ignored in root for ignored in ['.git', 'build', 'managed_components', 'components\esp-sr', 'graphify-out']):
            continue
            
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext in extensions:
                filepath = os.path.join(root, file)
                if process_file(filepath):
                    print(f"Đã làm sạch: {filepath}")
                    count += 1
                    
    print(f"\nĐã làm sạch toàn bộ {count} tệp.")

if __name__ == '__main__':
    target_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
    print(f"Bắt đầu dọn dẹp comment tại: {target_dir}")
    main(target_dir)

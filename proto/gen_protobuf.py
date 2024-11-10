import os
import sys


def gen_protobuf(proto_file):
    # 获取当前脚本的绝对路径
    proto_dir = os.path.dirname(os.path.abspath(__file__))

    # 构建 protoc 命令
    protoc_command_0 = (
        f'protoc.exe -I="{proto_dir}" --grpc_out="{proto_dir}" '
        f'--plugin=protoc-gen-grpc="C:\\env\\vcpkg\\packages\\grpc_x64-windows\\tools\\grpc\\grpc_cpp_plugin.exe" '
        f'"{proto_file}"'
    )

    # 打印命令以便调试
    print(f"Executing command: {protoc_command_0}")

    # 执行命令
    os.system(protoc_command_0)

    protoc_command_1 = f'protoc.exe --cpp_out="{proto_dir}" ' f'"{proto_file}"'

    # 打印命令以便调试
    print(f"Executing command: {protoc_command_1}")
    os.system(protoc_command_1)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_protobuf.py <proto_file>")
        sys.exit(1)

    proto_file = sys.argv[1]
    gen_protobuf(proto_file)

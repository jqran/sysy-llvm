from pathlib import Path
import sys,os
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QPlainTextEdit,
    QFileDialog, QAction, QTabWidget, QSplitter, QMessageBox,
    QTreeView, QToolButton
)
from PyQt5.QtCore import Qt, QDir, pyqtSignal, QRegExp, QTimer
from PyQt5.QtWidgets import QFileSystemModel
from qtconsole.rich_jupyter_widget import RichJupyterWidget
from qtconsole.manager import QtKernelManager
from PyQt5.QtGui import (
    QTextCharFormat, QFont, QSyntaxHighlighter, QColor, QTextCursor
)
from tree_sitter import Language, Parser


QUERY_PATH = Path("highlights.scm")
QUERY_STR = QUERY_PATH.read_text()

# 新增Tree-sitter高亮器
import tree_sitter_python as tspython
from tree_sitter import Language, Parser
PY_LANGUAGE = Language(tspython.language())
# 新增Tree-sitter高亮器
class TreeSitterHighlighter(QSyntaxHighlighter):
    def __init__(self, document):
        super().__init__(document)

        
        # 加载Tree-sitter Python语法（需要提前编译）
        self.language = PY_LANGUAGE
        self.parser = Parser(PY_LANGUAGE)        
        # self.parser.set_language(self.language)
        
        # 高亮规则配置
        self.highlight_styles = {
            # 类型: (颜色, 加粗)
            'keyword': (QColor(197, 134, 192), True),
            'string': (QColor(206, 145, 120), False),
            'comment': (QColor(98, 151, 85), False),
            'number': (QColor(181, 206, 168), False),
            'identifier': (QColor(181, 206, 168), False),
            'function': (QColor(78, 201, 176), True),
            'class': (QColor(255, 203, 107), True),
        }
        
        # 定义Tree-sitter查询模式
        self.query = self.language.query(QUERY_STR)
        
        # 使用定时器延迟解析以提高性能
        self.parse_timer = QTimer()
        self.parse_timer.setSingleShot(True)
        self.parse_timer.timeout.connect(self.parse)
        
        # 初始化时立即解析
        self.parse()


    def map_capture_to_style(self, capture_name: str):
        for key in self.highlight_styles:
            if capture_name == key or capture_name.startswith(f"{key}."):
                return self.highlight_styles[key]
        return None  # 默认无高亮

    # def parse(self):
    #     self.highlight_nodes = []
    #     text = self.document().toPlainText()
    #     tree = self.parser.parse(bytes(text, "utf-8"))
    #     captures = self.query.captures(tree.root_node)

    #     for node, capture_name in captures:
    #         start = node.start_byte
    #         end = node.end_byte
    #         start_char = self.byte_to_char_pos(text, start)
    #         end_char = self.byte_to_char_pos(text, end)
    #         style = self.map_capture_to_style(capture_name)
    #         if style:
    #             color, bold = style
    #             fmt = QTextCharFormat()
    #             fmt.setForeground(color)
    #             if bold:
    #                 fmt.setFontWeight(QFont.Bold)
    #             self.highlight_nodes.append((start_char, end_char, fmt))
    #     self.rehighlight()

    def parse(self):
        """解析整个文档并存储高亮信息"""
        
        # 清除之前的高亮信息
        self.highlight_nodes = []
        # 获取当前文本
        text = self.document().toPlainText()
        # 解析文本
        tree = self.parser.parse(bytes(text, "utf-8"))
        # 执行查询并存储结果
        captures = self.query.captures(tree.root_node)
        # print("Capture keys:", captures.keys())
        # print("First item:", list(captures.items())[0])
        # 正确遍历字典结构

        for tag_name, nodes in captures.items():  # 遍历字典项
            for node in nodes:                    # 遍历该标签对应的节点列表
                start = node.start_byte
                end = node.end_byte
                # 将字节位置转换为字符位置
                start_char = self.byte_to_char_pos(text, start)
                end_char = self.byte_to_char_pos(text, end)
                style = self.map_capture_to_style(tag_name)
                if style:
                    color, bold = style
                    fmt = QTextCharFormat()
                    fmt.setForeground(color)
                    if bold:
                        fmt.setFontWeight(QFont.Bold)
                    self.highlight_nodes.append((start_char, end_char, fmt))
            self.rehighlight()
        #         if style := self.highlight_styles.get(tag_name):
        #             color, bold = style
        #             fmt = QTextCharFormat()
        #             fmt.setForeground(color)
        #             if bold:
        #                 fmt.setFontWeight(QFont.Bold)
        #             self.highlight_nodes.append((start_char, end_char, fmt))
        # self.rehighlight()
        

        # self.rehighlight()
        
        
    def byte_to_char_pos(self, text, byte_pos):
        """将字节位置转换为字符位置"""
        return len(text.encode('utf-8')[:byte_pos].decode('utf-8', errors='ignore'))

    def highlightBlock(self, text):
        """应用高亮到当前文本块"""
        block = self.currentBlock()
        block_start = block.position()
        block_length = block.length()
        block_end = block_start + block_length

        for start, end, fmt in self.highlight_nodes:
            # 判断是否在当前块范围内
            if end <= block_start or start >= block_end:
                continue
            
            # 计算块内相对位置
            start_in_block = max(0, start - block_start)
            end_in_block = min(end - block_start, block_length)
            length = end_in_block - start_in_block
            
            if length > 0:
                self.setFormat(start_in_block, length, fmt)

    def contentsChange(self, position, chars_removed, chars_added):
        """文档内容变化时触发延迟解析"""
        self.parse_timer.start(100)  # 500毫秒延迟

# 新增资源管理器组件
class ResourceManager(QWidget):
    file_double_clicked = pyqtSignal(str)  # 定义双击文件信号

    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)
        self.setMinimumWidth(200)
        
        # 创建文件系统树
        self.model = QFileSystemModel()
        self.model.setRootPath(QDir.currentPath())  # 设置根目录为当前路径
        
        self.tree = QTreeView()
        self.tree.setModel(self.model)
        self.tree.setRootIndex(self.model.index(QDir.currentPath()))  # 显示当前目录
        
        # 设置显示效果
        self.tree.setHeaderHidden(True)                # 隐藏标题
        self.tree.setColumnHidden(1, True)             # 隐藏大小列
        self.tree.setColumnHidden(2, True)             # 隐藏类型列
        self.tree.setColumnHidden(3, True)             # 隐藏修改时间列
        self.tree.doubleClicked.connect(self.on_double_click)
        
        layout.addWidget(self.tree)

    def on_double_click(self, index):
        path = self.model.filePath(index)
        if os.path.isfile(path):  # 仅处理文件
            self.file_double_clicked.emit(path)

class TerminalWidget(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout(self)
        self.kernel_manager = QtKernelManager()
        self.kernel_manager.start_kernel()
        self.kernel_client = self.kernel_manager.client()
        self.kernel_client.start_channels()

        self.terminal = RichJupyterWidget()
        self.terminal.kernel_client = self.kernel_client
        self.terminal.kernel_manager = self.kernel_manager
        layout.addWidget(self.terminal)

    def execute(self, cmd):
        self.terminal.execute(cmd)

    def cleanup(self):
        self.kernel_client.stop_channels()
        self.kernel_manager.shutdown_kernel()


# 修改EditorWidget使用新高亮器
class EditorWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.current_file = None
        layout = QVBoxLayout(self)
        self.editor = QPlainTextEdit()
        self.editor.setStyleSheet("""
            font-family: Consolas; 
            font-size: 12pt; 
            background-color: #1e1e1e; 
            color: #d4d4d4;
        """)
        
        # 使用Tree-sitter高亮器
        self.highlighter = TreeSitterHighlighter(self.editor.document())
        
        # 监听文档变化
        self.editor.document().contentsChange.connect(
            self.highlighter.contentsChange
        )
        
        layout.addWidget(self.editor)

    # 其余方法保持不变...
    def load_file(self, path):
        with open(path, 'r') as f:
            content = f.read()
        self.editor.setPlainText(content)
        self.current_file = path

    def save_file(self, path=None):
        if path is None:
            path = self.current_file
        if path:
            with open(path, 'w') as f:
                f.write(self.editor.toPlainText())
            self.current_file = path
            return True
        return False


# class CodeEditor(QMainWindow):
#     def __init__(self):
#         super().__init__()
#         self.setWindowTitle("PyQt VSCode 风格编辑器")
#         self.resize(1000, 700)

#         # 主区域分割器
#         splitter = QSplitter(Qt.Vertical)
#         self.setCentralWidget(splitter)

#         # 编辑器标签页
#         self.editor_tabs = QTabWidget()
#         splitter.addWidget(self.editor_tabs)

#         # 终端标签页
#         self.terminal_tabs = QTabWidget()
#         splitter.addWidget(self.terminal_tabs)

#         add_terminal_btn = QToolButton()
#         add_terminal_btn.setText('+')
#         add_terminal_btn.clicked.connect(self.add_terminal_tab)
#         self.terminal_tabs.setCornerWidget(add_terminal_btn, Qt.TopRightCorner)
#         self.editor_tabs.setTabsClosable(True)
#         self.editor_tabs.tabCloseRequested.connect(self.close_editor_tab)
#         self.terminal_tabs.setTabsClosable(True)
#         self.terminal_tabs.tabCloseRequested.connect(self.close_terminal_tab)
#         splitter.setSizes([500, 200])  # 编辑器占上方大部分

#         self.init_menubar()
#         self.add_editor_tab()
#         self.add_terminal_tab()
# 修改后的主窗口类
class CodeEditor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("PyQt VSCode 风格编辑器")
        self.resize(1200, 700)  # 加宽窗口

        # 主水平分割器（左侧资源管理器，右侧编辑器+终端）
        main_splitter = QSplitter(Qt.Horizontal)
        self.setCentralWidget(main_splitter)

        # 左侧资源管理器
        self.resource_manager = ResourceManager()
        self.resource_manager.file_double_clicked.connect(self.open_file_from_explorer)
        main_splitter.addWidget(self.resource_manager)

        # 右侧垂直分割器
        right_splitter = QSplitter(Qt.Vertical)
        
        # 编辑器标签页
        self.editor_tabs = QTabWidget()
        right_splitter.addWidget(self.editor_tabs)

        # 终端标签页（添加加号按钮）
        self.terminal_tabs = QTabWidget()
        add_terminal_btn = QToolButton()
        add_terminal_btn.setText('+')
        add_terminal_btn.clicked.connect(self.add_terminal_tab)
        self.terminal_tabs.setCornerWidget(add_terminal_btn, Qt.TopRightCorner)
        right_splitter.addWidget(self.terminal_tabs)
        self.editor_tabs.setTabsClosable(True)
        self.editor_tabs.tabCloseRequested.connect(self.close_editor_tab)
        self.terminal_tabs.setTabsClosable(True)
        self.terminal_tabs.tabCloseRequested.connect(self.close_terminal_tab)
        right_splitter.setSizes([500, 200])
        main_splitter.addWidget(right_splitter)
        main_splitter.setSizes([200, 1000])  # 左侧资源管理器宽度200

        self.init_menubar()
        self.add_editor_tab()
        self.add_terminal_tab()

    # 新增方法：从资源管理器打开文件
    def open_file_from_explorer(self, path):
        # 检查是否已经打开
        for i in range(self.editor_tabs.count()):
            editor = self.editor_tabs.widget(i)
            if editor.current_file == path:
                self.editor_tabs.setCurrentIndex(i)
                return
        
        # 新建标签页打开
        self.add_editor_tab()
        editor = self.current_editor()
        editor.load_file(path)
        self.editor_tabs.setTabText(self.editor_tabs.currentIndex(), os.path.basename(path))

    def init_menubar(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu("文件")

        file_menu.addAction("新建文件", self.add_editor_tab)
        file_menu.addAction("打开", self.open_file)
        file_menu.addAction("保存", self.save_file)
        file_menu.addAction("另存为", self.save_file_as)
        file_menu.addAction("关闭当前文件", self.close_editor_tab)

        run_menu = menubar.addMenu("运行")
        run_menu.addAction("执行当前文件", self.run_current_file)
        run_menu.addAction("新建终端", self.add_terminal_tab)

    def current_editor(self):
        return self.editor_tabs.currentWidget()

    def current_terminal(self):
        return self.terminal_tabs.currentWidget()

    def add_editor_tab(self):
        editor = EditorWidget()
        index = self.editor_tabs.addTab(editor, "未命名")
        self.editor_tabs.setCurrentIndex(index)


    def close_editor_tab(self, index):
        if self.editor_tabs.count() <= 1:
            QMessageBox.warning(self, "提示", "至少保留一个标签页")
            return
        self.editor_tabs.removeTab(index)

    def open_file(self):
        path, _ = QFileDialog.getOpenFileName(self, "打开文件")
        if path:
            editor = self.current_editor()
            editor.load_file(path)
            self.editor_tabs.setTabText(self.editor_tabs.currentIndex(), path.split("/")[-1])

    def save_file(self):
        editor = self.current_editor()
        if editor.current_file:
            editor.save_file()
        else:
            self.save_file_as()

    def save_file_as(self):
        path, _ = QFileDialog.getSaveFileName(self, "另存为")
        if path:
            editor = self.current_editor()
            editor.save_file(path)
            self.editor_tabs.setTabText(self.editor_tabs.currentIndex(), path.split("/")[-1])

    def run_current_file(self):
        editor = self.current_editor()
        terminal = self.current_terminal()
        if not editor.current_file:
            self.save_file()
        if editor.current_file and terminal:
            cmd = f'!./your_compiler "{editor.current_file}"'
            terminal.execute(cmd)

    def add_terminal_tab(self):
        terminal = TerminalWidget()
        index = self.terminal_tabs.addTab(terminal, f"终端{self.terminal_tabs.count() + 1}")
        self.terminal_tabs.setCurrentIndex(index)

    def close_terminal_tab(self, index):
        self.terminal_tabs.removeTab(index)

    def closeEvent(self, event):
        for i in range(self.terminal_tabs.count()):
            term = self.terminal_tabs.widget(i)
            term.cleanup()
        super().closeEvent(event)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = CodeEditor()
    win.show()
    sys.exit(app.exec_())

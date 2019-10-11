using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace PDBManager
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
            //Application.Run(new LoginForm());  
          //Application.Run(new NewTableForm());
            //Application.Run(new QueryForm());
            //Application.Run(new MessageForm("请选择要删除的列","错误", MessageBoxIcon.Error,   true, false));
        }
    }
}

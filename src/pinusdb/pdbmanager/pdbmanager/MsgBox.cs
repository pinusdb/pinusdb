using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PDBManager
{
  class MsgBox
  {
    public static void ShowError(string msg)
    {
      MessageForm msgForm = new PDBManager.MessageForm(msg, 
        "错误", MessageBoxIcon.Error, true, false);
      msgForm.StartPosition = FormStartPosition.CenterParent;
      msgForm.ShowDialog();
      msgForm.Close();
    }

    public static void ShowInfo(string msg)
    {
      MessageForm msgForm = new MessageForm(msg, "信息", MessageBoxIcon.Information, true, false);
      msgForm.StartPosition = FormStartPosition.CenterParent;
      msgForm.ShowDialog();
      msgForm.Close();
    }

    public static DialogResult ShowQuestion (string msg)
    {
      MessageForm msgForm = new MessageForm(msg, "确认", MessageBoxIcon.Question, true, true);
      msgForm.StartPosition = FormStartPosition.CenterParent;
      msgForm.ShowDialog();
      msgForm.Close();
      return msgForm.DialogResult;
    }

  }
}

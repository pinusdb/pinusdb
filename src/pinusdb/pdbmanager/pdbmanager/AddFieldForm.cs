using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PDBManager
{
  public partial class AddFieldForm : Form
  {
    public string FieldName { get; private set; }
    public string FieldType { get; private set; }

    public AddFieldForm()
    {
      InitializeComponent();
    }

    public AddFieldForm(string fieldName, string fieldType)
    {
      InitializeComponent();
      this.Text = "修改字段";
      this.txbFieldName.Text = fieldName;
      this.cmbFieldType.Text = fieldType;
    }

    private void btnAccept_Click(object sender, EventArgs e)
    {
      if (!NameTool.ValidFieldName(txbFieldName.Text.Trim().ToLower()))
      {
        MessageForm msgForm = new MessageForm("非法的字段名!", "错误", MessageBoxIcon.Error, true, false);
        msgForm.StartPosition = FormStartPosition.CenterParent;
        msgForm.ShowDialog();
        return;
      }

      this.FieldName = this.txbFieldName.Text.Trim().ToLower();
      this.FieldType = this.cmbFieldType.Text.Trim().ToLower();

      this.DialogResult = DialogResult.OK;
      this.Close();
    }

    private void btnCancel_Click(object sender, EventArgs e)
    {
      this.DialogResult = DialogResult.Cancel;
      this.Close();
    }

    private void AddFieldForm_Load(object sender, EventArgs e)
    {
      if (string.IsNullOrEmpty(this.cmbFieldType.Text))
      {
        this.cmbFieldType.Text = this.cmbFieldType.Items[0].ToString();
      }
    }

    private void txbFieldName_KeyUp(object sender, KeyEventArgs e)
    {
      if (e.KeyCode == Keys.Enter)
      {
        this.cmbFieldType.Focus();
      }
    }

    private void cmbFieldType_KeyUp(object sender, KeyEventArgs e)
    {
      if (e.KeyCode == Keys.Enter)
      {
        this.btnAccept.Focus();
      }
    }

  }
}

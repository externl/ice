// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceVBConfigurationDialog : Form, IceConfigurationDialog
    {
        public IceVBConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;
            
            iceHomeView.init(this, _project);

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                setEnabled(enabled);
                chkEnableBuilder.Checked = enabled;
                load();
                _initialized = true;
            }
        }

#region IceConfigurationDialog interface

        public bool editingIncludeDir()
        {
            return false;
        }

        public void endEditIncludeDir(bool saveChanges)
        {
        }

        public void needSave()
        {
            if(_initialized)
            {
                btnApply.Enabled = hasUnsavedChanges();
            }
        }

        public void unsetCancelButton()
        {
            CancelButton = null;
        }

        public void setCancelButton()
        {
            CancelButton = btnCancel;
        }

#endregion IceConfigurationDialog interface
        
        private void load()
        {
            if(_project == null)
            {
                return;
            }
            Cursor = Cursors.WaitCursor;
            iceHomeView.load();

            loadComponents();
            btnApply.Enabled = false;
            Cursor = Cursors.Default;  
        }

        private void loadComponents()
        {
            ComponentList selectedComponents = Util.getIceDotNetComponents(_project);
            string[] dotNetNames = Util.getDotNetNames();
            foreach(String s in dotNetNames)
            {
                if(selectedComponents.Contains(s))
                {
                    checkComponent(s, true);
                }
                else
                {
                    checkComponent(s, false);
                }
            }
        }

        private void checkComponent(String component, bool check)
        {
            switch(component)
            {
                case "Glacier2":
                {
                    chkGlacier2.Checked = check;
                    break;
                }
                case "Ice":
                {
                    chkIce.Checked = check;
                    break;
                }
                case "IceBox":
                {
                    chkIceBox.Checked = check;
                    break;
                }
                case "IceGrid":
                {
                    chkIceGrid.Checked = check;
                    break;
                }
                case "IcePatch2":
                {
                    chkIcePatch2.Checked = check;
                    break;
                }
                case "IceSSL":
                {
                    chkIceSSL.Checked = check;
                    break;
                }
                case "IceStorm":
                {
                    chkIceStorm.Checked = check;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        private void chkEnableBuilder_CheckedChanged(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(_initialized)
                {
                    _initialized = false;
                    setEnabled(chkEnableBuilder.Checked);

                    ComponentList components;
                    if(chkEnableBuilder.Checked)
                    {
                        //
                        // Enable the components that were previously enabled, if any.
                        //
                        components = new ComponentList(Util.getProjectProperty(_project, Util.PropertyIceComponents));

                        //
                        // If there isn't a previous set of components, we enable the default components.
                        //
                        if(components.Count == 0)
                        {
                            components.Add("Ice");
                        }
                    }
                    else
                    {
                        components = iceComponents();
                    }

                    // Enable / Disable the given set of components
                    for(int i = 0; i < components.Count; ++i)
                    {
                        checkComponent(components[i], chkEnableBuilder.Checked);
                    }

                    chkEnableBuilder.Enabled = true;
                    _initialized = true;
                    needSave();
                }
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }
        
        private void setEnabled(bool enabled)
        {
            iceHomeView.setEnabled(enabled);
            chkGlacier2.Enabled = enabled;
            chkIce.Enabled = enabled;
            chkIceBox.Enabled = enabled;
            chkIceGrid.Enabled = enabled;
            chkIcePatch2.Enabled = enabled;
            chkIceSSL.Enabled = enabled;
            chkIceStorm.Enabled = enabled;
        }

        private void componentChanged(string name, bool value, bool development)
        {
            if(value)
            {
                if(!Util.addDotNetReference(_project, name, Util.getIceHome(_project), development))
                {
                    checkComponent(name, false);
                }
            }
            else
            {
                Util.removeDotNetReference(_project, name);
            }
        }

        private void component_Changed(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(editingIncludeDir())
                {
                    endEditIncludeDir(true);
                }
                needSave();
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // Apply unsaved changes, returns true if new settings are all applied correctly,
        // otherwise returns false.
        //
        private bool apply()
        {
            if(!hasUnsavedChanges())
            {
                return true; // Nothing to do.
            }

            try
            {
                if(editingIncludeDir())
                {
                    endEditIncludeDir(true);
                }

                //
                // This must be the first setting to be updated, as other settings cannot be
                // updated if the add-in is disabled.
                //
                if(chkEnableBuilder.Checked && !Util.isSliceBuilderEnabled(_project))
                {
                    Util.addBuilderToProject(_project, iceComponents());
                    _initialized = false;
                    load();
                    _initialized = true;
                    // There aren't other changes at this point, we have just enabled the add-in.
                    return true;
                }

                bool changed = false;
                if(!iceHomeView.apply(ref changed))
                {
                    //
                    // We don't apply other changes until that error is fixed.
                    // This should not happen in normal circumstances, as we check
                    // that Ice Home is valid when the corresponding field is changed.
                    // This could happen if the directory is removed after the field
                    // was edited and apply was clicked.
                    //
                    return false;
                }

                bool development = Util.developmentMode(_project);
                if(chkGlacier2.Checked != Util.hasDotNetReference(_project, "Glacier2"))
                {
                    componentChanged("Glacier2", chkGlacier2.Checked, development);
                }
                if(chkIce.Checked != Util.hasDotNetReference(_project, "Ice"))
                {
                    componentChanged("Ice", chkIce.Checked, development);
                }
                if(chkIceBox.Checked != Util.hasDotNetReference(_project, "IceBox"))
                {
                    componentChanged("IceBox", chkIceBox.Checked, development);
                }
                if(chkIceGrid.Checked != Util.hasDotNetReference(_project, "IceGrid"))
                {
                    componentChanged("IceGrid", chkIceGrid.Checked, development);
                }
                if(chkIcePatch2.Checked != Util.hasDotNetReference(_project, "IcePatch2"))
                {
                    componentChanged("IcePatch2", chkIcePatch2.Checked, development);
                }
                if(chkIceSSL.Checked != Util.hasDotNetReference(_project, "IceSSL"))
                {
                    componentChanged("IceSSL", chkIceSSL.Checked, development);
                }
                if(chkIceStorm.Checked != Util.hasDotNetReference(_project, "IceStorm"))
                {
                    componentChanged("IceStorm", chkIceStorm.Checked, development);
                }

                //
                // This must be the last setting to be updated, as we want to update 
                // all other settings and that isn't possible if the builder is disabled.
                //
                if(!chkEnableBuilder.Checked && Util.isSliceBuilderEnabled(_project))
                {
                    Util.removeBuilderFromProject(_project);
                    _initialized = false;
                    load();
                    _initialized = true;
                }

                return true;
            }
            finally
            {
                needSave(); // Call needSave to update apply button status
            }
        }

        private bool hasUnsavedChanges()
        {
            if(chkEnableBuilder.Checked != Util.isSliceBuilderEnabled(_project))
            {
                return true;
            }

            //
            // If the builder is disabled, we are not interested in the other settings
            // to compute changes.
            //
            if(!Util.isSliceBuilderEnabled(_project))
            {
                return false;
            }

            if(iceHomeView.hasUnsavedChanges())
            {
                return true;
            }

            // Ice libraries
            if(chkGlacier2.Checked != Util.hasDotNetReference(_project, "Glacier2"))
            {
                return true;
            }
            if(chkIce.Checked != Util.hasDotNetReference(_project, "Ice"))
            {
                return true;
            }
            if(chkIceBox.Checked != Util.hasDotNetReference(_project, "IceBox"))
            {
                return true;
            }
            if(chkIceGrid.Checked != Util.hasDotNetReference(_project, "IceGrid"))
            {
                return true;
            }
            if(chkIcePatch2.Checked != Util.hasDotNetReference(_project, "IcePatch2"))
            {
                return true;
            }
            if(chkIceSSL.Checked != Util.hasDotNetReference(_project, "IceSSL"))
            {
                return true;
            }
            if(chkIceStorm.Checked != Util.hasDotNetReference(_project, "IceStorm"))
            {
                return true;
            }
            return false;
        }

        private ComponentList iceComponents()
        {
            ComponentList components = new ComponentList();

            if(chkGlacier2.Checked)
            {
                components.Add("Glacier2");
            }
            if(chkIce.Checked)
            {
                components.Add("Ice");
            }
            if(chkIceBox.Checked)
            {
                components.Add("IceBox");
            }
            if(chkIceGrid.Checked)
            {
                components.Add("IceGrid");
            }
            if(chkIcePatch2.Checked)
            {
                components.Add("IcePatch2");
            }
            if(chkIceSSL.Checked)
            {
                components.Add("IceSSL");
            }
            if(chkIceStorm.Checked)
            {
                components.Add("IceStorm");
            }
            return components;
        }

        private void formClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                if(!hasUnsavedChanges())
                {
                    return;
                }

                if(!Util.warnUnsavedChanges(this))
                {
                    e.Cancel = true;
                    return;
                }
            }
            catch(Exception ex)
            {
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(apply())
                {
                    Close();
                }
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                apply();
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                Close();
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private bool _initialized;
        private Project _project;
    }
}

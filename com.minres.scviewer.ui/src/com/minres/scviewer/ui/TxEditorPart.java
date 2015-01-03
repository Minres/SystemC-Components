/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui;

import java.io.File;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.FileStoreEditorInput;
import org.eclipse.ui.internal.ide.dialogs.IFileStoreFilter;
import org.eclipse.ui.part.EditorPart;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;
import org.eclipse.ui.views.properties.IPropertySheetPage;
import org.eclipse.ui.views.properties.tabbed.ITabbedPropertySheetPageContributor;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.InvalidSyntaxException;
import org.osgi.framework.ServiceReference;

import com.minres.scviewer.database.ITrDb;
import com.minres.scviewer.database.ITrStream;
import com.minres.scviewer.database.ITransactionDbFactory;
import com.minres.scviewer.ui.swt.TxDisplay;
import com.minres.scviewer.ui.views.TxOutlinePage;

public class TxEditorPart extends EditorPart implements ITabbedPropertySheetPageContributor {

	public static final String ID = "com.minres.scviewer.ui.TxEditorPart"; //$NON-NLS-1$

	public static final String WAVE_ACTION_ID = "com.minres.scviewer.ui.action.AddToWave";

	private TxDisplay txDisplay;
	
	/** This is the root of the editor's model. */
	private ITrDb database;

	private Composite myParent;

	public TxEditorPart() {
	}

	/**
	 * Create contents of the editor part.
	 * @param parent
	 */
	@Override
	public void createPartControl(Composite parent) {
		myParent=parent;
		/** Add handlers for global actions (delete, etc) */
//		IActionBars actionBars = getEditorSite().getActionBars();
//		actionBars.setGlobalActionHandler(WAVE_ACTION_ID,	new Action() {
//			@Override
//			public void runWithEvent(Event event) {
//				System.out.println("AddToWave with event");
//			}
//			
//			@Override
//			public void run() {
//				System.out.println("AddToWave");
//			}
//		});
		
		txDisplay = new TxDisplay(parent);
		if(database!=null) database.addPropertyChangeListener(txDisplay); 
		getSite().setSelectionProvider(txDisplay);
		if(getEditorInput()!=null && ((TxEditorInput) getEditorInput()).getStreamNames().size()>0 && database!=null){
			if(MessageDialog.openConfirm(parent.getShell(), "Confirm", "Do you want the restore last state of the wave form?"))
				for(String streamName:((TxEditorInput) getEditorInput()).getStreamNames()){
					ITrStream stream = database.getStreamByName(streamName);
					if(stream!=null)
						txDisplay.addStream(stream);
				}
		}
	}

	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.EditorPart#setInput(org.eclipse.ui.IEditorInput)
	 */
	protected void setInput(IEditorInput input) {
		super.setInput(input);
		if(input instanceof IFileEditorInput){
			if(!(input instanceof TxEditorInput))
				super.setInput(new TxEditorInput(((IFileEditorInput)input).getFile()));
			try {
				IPath location = ((IFileEditorInput) input).getFile().getLocation();
				if (location != null)
					getTrDatabase(location.toFile());
				setPartName(((IFileEditorInput) input).getFile().getName());
			} catch (Exception e) {
				handleLoadException(e);
			}
		} else if(input instanceof FileStoreEditorInput){
			try {
				//database.load(((FileStoreEditorInput) input).getURI().toURL().openStream());
				File file=new File(((FileStoreEditorInput) input).getURI().getPath());
				getTrDatabase(file);
				setPartName(((FileStoreEditorInput) input).getName());
			} catch (Exception e) {
				handleLoadException(e);
			}
		}
	}

	protected void getTrDatabase(File file) {
		try {
			BundleContext context = FrameworkUtil.getBundle(this.getClass()).getBundleContext();
			ServiceReference<?>[] serviceReferences = context.getServiceReferences(ITransactionDbFactory.class.getName(), null);
			if(serviceReferences!=null){
				for(ServiceReference<?> serviceReference:serviceReferences){
					database = ((ITransactionDbFactory) context.getService(serviceReference)).createDatabase(file);
					if(database!=null){
						if(txDisplay !=null) database.addPropertyChangeListener(txDisplay);
						return;
					}
				}
			}
		} catch (Exception e) {
		}
		MessageDialog.openError(PlatformUI.getWorkbench().getDisplay().getActiveShell(),
				"Error loading database", "Could not find an usable and applicable database loader implementation");
		database=null;
//		if(TxEditorPlugin.getDefault().getTransactionDbFactory()!=null){
//			database = TxEditorPlugin.getDefault().getTransactionDbFactory().createDatabase();
//			if(txDisplay !=null) database.addPropertyChangeListener(txDisplay);
//		} else {
//			MessageDialog.openError(PlatformUI.getWorkbench().getDisplay().getActiveShell(),
//					"Error loading database", "Could not find database loader implementation");
//			database=null;
//		}
	}

	private void handleLoadException(Exception e) {
		MessageDialog.openError(PlatformUI.getWorkbench().getDisplay().getActiveShell(),
				"Error loading database", e.getMessage());
		database = null;
	}

	@Override
	public void setFocus() {
		myParent.setFocus();
	}

	@Override
	public void doSave(IProgressMonitor monitor) {
	}

	@Override
	public void doSaveAs() {
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Object getAdapter(Class type) {
		if (type == IContentOutlinePage.class) // outline page
			return new TxOutlinePage(this);
		else if (type == IPropertySheetPage.class) // use tabbed property sheet instead of standard one
            return new TabbedPropertySheetPage(this);
		return super.getAdapter(type);
	}

	public ITrDb getModel() {
		return database;
	}

	@Override
	public void init(IEditorSite site, IEditorInput input)
			throws PartInitException {
		// Initialize the editor part
		setSite(site);
		setInput(input);
	}

	@Override
	public boolean isDirty() {
		return false;
	}

	@Override
	public boolean isSaveAsAllowed() {
		return false;
	}

	public ITrDb getDatabase() {
		return database;
	}

	public void addStreamToList(ITrStream stream){
		if(getEditorInput() instanceof TxEditorInput && !((TxEditorInput) getEditorInput()).getStreamNames().contains(stream.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().add(stream.getFullName());
			txDisplay.addStream(stream);
		} else
			txDisplay.addStream(stream);
			
	}
	
	public void addStreamsToList(ITrStream[] streams){
		for(ITrStream stream:streams)
			addStreamToList(stream);
	}

	public void removeStreamFromList(ITrStream stream){
		if(getEditorInput() instanceof TxEditorInput && ((TxEditorInput) getEditorInput()).getStreamNames().contains(stream.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().remove(stream.getFullName());
			txDisplay.removeStream(stream);
		} else
			txDisplay.removeStream(stream);
	}
	
	public void removeStreamsFromList(ITrStream[] streams){
		for(ITrStream stream:streams)
			removeStreamFromList(stream);
	}
	
	public List<ITrStream> getStreamList(){
		return txDisplay.getStreamList();
	}

	public void setSelection(final ISelection selection){
		myParent.getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				txDisplay.setSelection(selection);
			}
		});
	}
	
	@Override
	public String getContributorId() {
		return getSite().getId();
	}

}

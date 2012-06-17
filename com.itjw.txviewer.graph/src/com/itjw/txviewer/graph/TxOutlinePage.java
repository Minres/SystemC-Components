package com.itjw.txviewer.graph;

import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.ISelectionListener;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.part.IPageSite;
import org.eclipse.ui.views.contentoutline.ContentOutline;
import org.eclipse.ui.views.contentoutline.ContentOutlinePage;

import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.graph.actions.TxActionFactory;
import com.itjw.txviewer.graph.data.ITrHierNodeFacade;
import com.itjw.txviewer.graph.views.provider.TxDbTreeContentProvider;
import com.itjw.txviewer.graph.views.provider.TxDbTreeLabelProvider;

/**
 * Creates an outline pagebook for this editor.
 */
public class TxOutlinePage extends ContentOutlinePage implements  ISelectionListener, ISelectionProvider {

	private TxEditorPart editor;

	public TxOutlinePage(TxEditorPart editor) {
		this.editor = editor;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.eclipse.ui.part.IPage#createControl(org.eclipse.swt.widgets.Composite
	 * )
	 */
	public void createControl(Composite parent) {
		super.createControl(parent);
		TreeViewer contentOutlineViewer = getTreeViewer();
		contentOutlineViewer.addSelectionChangedListener(this);
		// Set up the tree viewer
		contentOutlineViewer.setContentProvider(new TxDbTreeContentProvider());
		contentOutlineViewer.setLabelProvider(new TxDbTreeLabelProvider());
		contentOutlineViewer.setInput(editor.getDatabase());
		// initialize context menu depending on the the selectec item
		MenuManager menuMgr = new MenuManager();
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager manager) {
				fillContextMenu(manager);
			}
		});
		Menu menu = menuMgr.createContextMenu(contentOutlineViewer.getControl());
		contentOutlineViewer.getTree().setMenu(menu);
		getSite().registerContextMenu("com.itjw.txviewer.graph.outline.contextmenu", menuMgr, contentOutlineViewer);
		// add me as selection listener
		getSite().getPage().addSelectionListener((ISelectionListener) this);
		//getSite().getPage().addSelectionListener("SampleViewId",(ISelectionListener)this);
		getSite().setSelectionProvider(this);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.IPage#dispose()
	 */
	public void dispose() {
		// dispose
		super.dispose();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.IPage#getControl()
	 */
	public Control getControl() {
		return getTreeViewer().getControl();
	}

	/**
	 * @see org.eclipse.ui.part.IPageBookViewPage#init(org.eclipse.ui.part.IPageSite)
	 */
	public void init(IPageSite pageSite) {
		super.init(pageSite);
		IActionBars bars = pageSite.getActionBars();
		String id = ActionFactory.PASTE.getId();
		bars.setGlobalActionHandler(id, editor.getActionRegistry().getAction(id));
		id = ActionFactory.DELETE.getId();
		bars.setGlobalActionHandler(id, editor.getActionRegistry().getAction(id));
	}

	private void fillContextMenu(IMenuManager menuMgr) {
		// initalize the context menu
		getTreeViewer().getSelection();
		ISelection selection = getTreeViewer().getSelection();
		Object obj = ((IStructuredSelection) selection).getFirstElement();
		menuMgr.add(TxActionFactory.getAction(TxActionFactory.ADD_TO_WAVE,  obj instanceof ITrStream));
		menuMgr.add(TxActionFactory.getAction(TxActionFactory.ADD_ALL_TO_WAVE,  true));
		menuMgr.add(TxActionFactory.getAction(TxActionFactory.REMOVE_FROM_WAVE,  obj instanceof ITrStream));
		menuMgr.add(TxActionFactory.getAction(TxActionFactory.REMOVE_ALL_FROM_WAVE,  true));
	}

	
	//ISelectionListener methods
	@Override
	public void selectionChanged(IWorkbenchPart part, ISelection selection) {
		if(!(part instanceof ContentOutline) && selection instanceof TrHierNodeSelection){
			System.out.println("Something selected");			
		}
	}
	
    /**
     * Returns the current selection for this provider.
     * 
     * @return the current selection
     */
    public ISelection getSelection() {
        if (getTreeViewer() == null) {
			return StructuredSelection.EMPTY;
		}
        return getTreeViewer().getSelection();
    }

	public void setSelection(ISelection selection){
	    if (getTreeViewer() != null) {
	    	getTreeViewer().setSelection(selection);
		}
	}
    /**
      * @see org.eclipse.jface.viewers.ISelectionProvider#selectionChanged(SelectionChangedEvent)
      */
     public void selectionChanged(SelectionChangedEvent anEvent) {
    	 // translate the tree selection
         ISelection selection = anEvent.getSelection();
         if (!selection.isEmpty()) {
             Object tmp = ((IStructuredSelection) selection).getFirstElement();
             if (tmp instanceof ITrHierNodeFacade) {
            	 fireSelectionChanged(new TrHierNodeSelection((ITrHierNodeFacade) tmp));
             }
         }
     }

}

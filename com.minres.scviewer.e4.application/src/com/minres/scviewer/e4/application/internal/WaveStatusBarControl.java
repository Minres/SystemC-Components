/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.e4.application.internal;

import java.util.List;

import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.di.UISynchronize;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.jface.action.ContributionItem;
import org.eclipse.jface.action.StatusLineManager;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CLabel;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;

public class WaveStatusBarControl extends StatusBarControl {

	public static final String STATUS_UPDATE="StatusUpdate";
	public static final String ZOOM_LEVEL="ZoomLevelUpdate";
	public static final String CURSOR_TIME="CursorPosUpdate";

	@Inject
	EModelService modelService;

	class TextContributionItem extends ContributionItem {

		final String labelString;
		final int width;
		CLabel label, text;
		private String content;

		public TextContributionItem(String labelString, int width) {
			super();
			this.labelString = labelString;
			this.width=width;
			content="";
		}

		@Override
		public void fill(Composite parent) {
			Composite box=new Composite(parent, SWT.NONE);
			box.setLayout(new GridLayout(2, false));
			label=new CLabel(box, SWT.SHADOW_NONE);	
			label.setText(labelString);
			text=new CLabel(box, SWT.SHADOW_IN);
			GridData layoutData=new GridData(SWT.DEFAULT, SWT.DEFAULT, true, false);
			layoutData.minimumWidth=width;
			text.setLayoutData(layoutData);
		}

		@Override
		public boolean isDynamic() {
			return true;
		}

		public void setText(String message){
			this.content=message;
			if(text!=null && !text.isDisposed()) text.setText(content);
		}

	}

	TextContributionItem zoomContribution, cursorContribution;

	@Inject
	public WaveStatusBarControl(UISynchronize sync) {
		super(sync);
		zoomContribution = new TextContributionItem("Z:", 150);
		cursorContribution = new TextContributionItem("C:", 120);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,cursorContribution);
		manager.appendToGroup(StatusLineManager.MIDDLE_GROUP, zoomContribution);
	}

	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION)@Optional Object obj){
		//		if(status!=null ) status.setText(obj==null?"":obj.toString());
		if(manager!=null ){
			if(obj instanceof List<?>){
				manager.setMessage(""+((List<?>)obj).size()+" Elements");
			} else 
				manager.setMessage(obj==null?"":obj.getClass().getSimpleName()+" selected");
		}
	}

	@Inject @Optional
	public void  getZoomEvent(@UIEventTopic(ZOOM_LEVEL) String text) {
		zoomContribution.setText(text);
	} 

	@Inject @Optional
	public void  getCursorEvent(@UIEventTopic(CURSOR_TIME) String text) {
		cursorContribution.setText(text);
	} 

}
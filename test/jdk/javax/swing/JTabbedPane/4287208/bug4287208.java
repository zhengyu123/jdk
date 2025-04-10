/*
 * Copyright (c) 2001, 2025, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * @test
 * @bug 4287208
 * @summary Tests if JTabbedPane's setEnabledAt properly renders bounds of Tabs
 * @library /java/awt/regtesthelpers
 * @build PassFailJFrame
 * @run main/manual bug4287208
*/

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;

public class bug4287208 implements ActionListener {

    static final String INSTRUCTIONS = """
        There are two tabs in the test window. Press the "Test" button 5 times.
        If this causes tabs to overlap at any time, the test FAILS, otherwise
        the test PASSES.
    """;

    static boolean state = true;
    static JTabbedPane jtp;

    public static void main(String[] args) throws Exception {
        PassFailJFrame.builder()
            .title("bug4287208 Test Instructions")
            .instructions(INSTRUCTIONS)
            .columns(40)
            .testUI(bug4287208::createUI)
            .build()
            .awaitAndCheck();
    }

    static JFrame createUI() {
        JFrame frame = new JFrame("bug4287208");

        JButton start = new JButton("Test");
        start.addActionListener(new bug4287208());
        JPanel buttonPanel = new JPanel();
        buttonPanel.add(start);
        frame.add(buttonPanel,BorderLayout.SOUTH);

        jtp = new JTabbedPane();
        jtp.addTab("Panel One", new JPanel());
        String s = System.getProperty("test.src",".") +
                   System.getProperty("file.separator") + "duke.gif";
        ImageIcon ii = new ImageIcon(s);
        jtp.addTab("Panel Two", ii, new JPanel());

        frame.add(jtp, BorderLayout.CENTER);
        frame.setSize(500, 300);
        return frame;
    }

    public void actionPerformed(ActionEvent evt) {
        jtp.setEnabledAt(0, state);
        jtp.setEnabledAt(1, !state);
        state = !state;
    }

}

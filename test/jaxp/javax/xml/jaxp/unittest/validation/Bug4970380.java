/*
 * Copyright (c) 2014, 2024, Oracle and/or its affiliates. All rights reserved.
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

package validation;

import javax.xml.validation.SchemaFactory;
import javax.xml.validation.ValidatorHandler;

import org.testng.Assert;
import org.testng.annotations.Test;
import org.xml.sax.SAXNotRecognizedException;

/*
 * @test
 * @bug 4970380
 * @library /javax/xml/jaxp/libs /javax/xml/jaxp/unittest
 * @run testng/othervm validation.Bug4970380
 * @summary Test validatorHandler.getFeature(...) with unknown name and "http://xml.org/sax/features/namespace-prefixes".
 */
public class Bug4970380 {

    @Test
    public void test1() throws Exception {
        SchemaFactory schemaFactory = SchemaFactory.newInstance("http://www.w3.org/2001/XMLSchema");
        ValidatorHandler validatorHandler = schemaFactory.newSchema().newValidatorHandler();

        try {
            validatorHandler.getFeature("unknown1234");
            Assert.fail("SAXNotRecognizedException was not thrown.");
        } catch (SAXNotRecognizedException e) {
            ; // expected
        }

        if (!validatorHandler.getFeature("http://xml.org/sax/features/namespace-prefixes")) {
            // as expected
            System.out.println("getFeature(namespace-prefixes): OK");
        } else {
            Assert.fail("Expected false, returned true.");
        }
    }
}

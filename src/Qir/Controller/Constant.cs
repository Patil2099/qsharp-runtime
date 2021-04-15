﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Quantum.Qir
{
    public static class Constant
    {
        // TODO: errors will be added as dependencies are implemented.
        public static class ErrorCode
        {
            public const string InternalError = "InternalError";
        }

        public static class FileExtension
        {
            public const string CppExtension = ".cpp";
            public const string BytecodeExtension = ".bc";
        }
    }
}
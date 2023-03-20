using OpenTK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TOBBETUROCKETRY
{
    class ObjImporter
    {

        public static void LoadModel(string filePath, out List<Vector3> vertices, out List<int> triangles, out List<Vector3> normals, out List<Vector2> texCoords)
        {
            vertices = new List<Vector3>();
            triangles = new List<int>();
            normals = new List<Vector3>();
            texCoords = new List<Vector2>();

            if (File.Exists(filePath))
            {

                string[] lines = File.ReadAllLines(filePath);

                for (int i = 0; i < lines.Length; i++)
                {
                    if (lines[i].Contains("v "))

                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0, z = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);

                        if (tokens.Length > 3)
                            float.TryParse(tokens[3], out z);

                        vertices.Add(new Vector3(x, y, z));
                    }
                    else if (lines[i].Contains("f "))
                    {
                        string[] tokens = lines[i].Split(' ');
                        for (int j = 1; j < tokens.Length; j++)
                        {
                            if (tokens[j].Contains('/'))
                            {
                                string[] subTokens = tokens[j].Split('/');

                                int index;
                                for(int a =0; a < subTokens.Length;a++)
                                {
                                    if(Int32.TryParse(subTokens[0], out index))
                                    {
                                        triangles.Add(index - 1);
                                    }
                                    
                                }
                                
                            }
                            else
                            {
                                int index;
                                if (Int32.TryParse(tokens[j], out index))
                                {
                                    triangles.Add(index - 1);
                                }

                            }

                        }

                        //for (int j = 1; j < tokens.Length; j += 2)
                        //{
                        //    int index = 0;
                        //    Int32.TryParse(tokens[j], out index);
                        //
                        //    triangles.Add(index - 1);
                        //}
                    }
                    else if (lines[i].Contains("vt "))
                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);

                        texCoords.Add(new Vector2(x, y));
                    }
                    else if (lines[i].Contains("vn "))
                    {
                        string[] tokens = lines[i].Split(' ');

                        float x = 0, y = 0, z = 0;

                        float.TryParse(tokens[1], out x);
                        float.TryParse(tokens[2], out y);

                        if (tokens.Length > 3)
                            float.TryParse(tokens[3], out z);

                        normals.Add(new Vector3(x, y, z));
                    }

                }

            }
            else
            {
                throw new System.Exception("OBJ file not found: " + filePath);
            }
        }

    }
}

